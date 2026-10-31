#include "common.h"
#include <urlmon.h>
#include <winhttp.h>
#include <cstdarg>
#include <cwchar>
#include <cwctype>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

HINSTANCE g_inst = nullptr;
int g_dpi = 96;

// ============================================================ strings

std::wstring W(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}

std::string U8(const std::wstring& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &s[0], n, nullptr, nullptr);
    return s;
}

std::wstring Fmt(const wchar_t* fmt, ...) {
    wchar_t buf[2048];
    va_list ap;
    va_start(ap, fmt);
    int n = vswprintf(buf, 2048, fmt, ap);
    va_end(ap);
    if (n < 0) buf[2047] = 0;
    return buf;
}

std::wstring Trim(const std::wstring& s) {
    size_t a = 0, b = s.size();
    while (a < b && iswspace(s[a])) ++a;
    while (b > a && iswspace(s[b - 1])) --b;
    return s.substr(a, b - a);
}

std::wstring Lower(std::wstring s) {
    if (!s.empty()) CharLowerBuffW(&s[0], (DWORD)s.size());
    return s;
}

std::wstring Upper(std::wstring s) {
    if (!s.empty()) CharUpperBuffW(&s[0], (DWORD)s.size());
    return s;
}

bool StartsWith(const std::wstring& s, const std::wstring& p) {
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

bool ContainsI(const std::wstring& hay, const std::wstring& needle) {
    if (needle.empty()) return true;
    return StrStrIW(hay.c_str(), needle.c_str()) != nullptr;
}

std::vector<std::wstring> Split(const std::wstring& s, wchar_t sep) {
    std::vector<std::wstring> out;
    size_t start = 0;
    for (;;) {
        size_t pos = s.find(sep, start);
        out.push_back(s.substr(start, pos == std::wstring::npos ? std::wstring::npos : pos - start));
        if (pos == std::wstring::npos) break;
        start = pos + 1;
    }
    return out;
}

std::wstring FormatSize(long long b) {
    if (b <= 0) return L"";
    static const wchar_t* units[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
    double v = (double)b;
    int i = 0;
    while (v >= 1024.0 && i < 4) { v /= 1024.0; ++i; }
    if (i == 0) return Fmt(L"%lld B", b);
    return Fmt(v < 10 ? L"%.1f %ls" : L"%.0f %ls", v, units[i]);
}

std::wstring FormatDuration(double sec) {
    if (sec <= 0) return L"";
    long long t = (long long)(sec + 0.5);
    int h = (int)(t / 3600), m = (int)(t % 3600 / 60), s = (int)(t % 60);
    if (h > 0) return Fmt(L"%d:%02d:%02d", h, m, s);
    return Fmt(L"%02d:%02d", m, s);
}

std::wstring FormatEta(double sec) {
    if (sec < 0) return L"";
    long long t = (long long)(sec + 0.5);
    if (t < 60) return Fmt(L"%lld초", t);
    if (t < 3600) return Fmt(L"%lld분 %02lld초", t / 60, t % 60);
    return Fmt(L"%lld시간 %lld분", t / 3600, t % 3600 / 60);
}

long long NowMs() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER u;
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    return (long long)(u.QuadPart / 10000ULL - 11644473600000ULL);
}

// ============================================================ files & paths

std::wstring ExeDir() {
    wchar_t buf[MAX_PATH * 2];
    DWORD n = GetModuleFileNameW(nullptr, buf, (DWORD)(sizeof buf / sizeof buf[0]));
    std::wstring p(buf, n);
    return PathDir(p);
}

std::wstring PathJoin(const std::wstring& a, const std::wstring& b) {
    if (a.empty()) return b;
    if (a.back() == L'\\' || a.back() == L'/') return a + b;
    return a + L"\\" + b;
}

std::wstring PathDir(const std::wstring& p) {
    size_t pos = p.find_last_of(L"\\/");
    return pos == std::wstring::npos ? L"" : p.substr(0, pos);
}

std::wstring PathName(const std::wstring& p) {
    size_t pos = p.find_last_of(L"\\/");
    return pos == std::wstring::npos ? p : p.substr(pos + 1);
}

std::wstring PathStem(const std::wstring& p) {
    std::wstring n = PathName(p);
    size_t dot = n.find_last_of(L'.');
    return dot == std::wstring::npos ? n : n.substr(0, dot);
}

bool FileExists(const std::wstring& p) {
    if (p.empty()) return false;
    DWORD a = GetFileAttributesW(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

bool DirExists(const std::wstring& p) {
    if (p.empty()) return false;
    DWORD a = GetFileAttributesW(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

long long FileSize(const std::wstring& p) {
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (!GetFileAttributesExW(p.c_str(), GetFileExInfoStandard, &d)) return 0;
    return ((long long)d.nFileSizeHigh << 32) | d.nFileSizeLow;
}

void EnsureDir(const std::wstring& p) {
    if (!p.empty() && !DirExists(p)) SHCreateDirectoryExW(nullptr, p.c_str(), nullptr);
}

bool DirWritable(const std::wstring& p) {
    if (p.empty() || !DirExists(p)) return false;
    std::wstring probe = PathJoin(p, Fmt(L".ytpc-%lu.tmp", GetTickCount()));
    HANDLE h = CreateFileW(probe.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    CloseHandle(h);
    return true;
}

std::wstring KnownFolder(REFKNOWNFOLDERID id) {
    PWSTR p = nullptr;
    std::wstring r;
    if (SUCCEEDED(SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, nullptr, &p)) && p) r = p;
    if (p) CoTaskMemFree(p);
    return r;
}

std::wstring AppDataDir() { return PathJoin(KnownFolder(FOLDERID_RoamingAppData), APP_DIRNAME); }
std::wstring LocalDataDir() { return PathJoin(KnownFolder(FOLDERID_LocalAppData), APP_DIRNAME); }

std::wstring SanitizeFileName(const std::wstring& name) {
    std::wstring r;
    for (wchar_t c : name) {
        switch (c) {
        case L'<': r += L'\xFF1C'; break;
        case L'>': r += L'\xFF1E'; break;
        case L':': r += L'\xFF1A'; break;
        case L'"': r += L'\xFF02'; break;
        case L'/': r += L'\xFF0F'; break;
        case L'\\': r += L'\xFF3C'; break;
        case L'|': r += L'\xFF5C'; break;
        case L'?': r += L'\xFF1F'; break;
        case L'*': r += L'\xFF0A'; break;
        default: r += (c < 32) ? L' ' : c; break;
        }
    }
    r = Trim(r);
    while (!r.empty() && (r.back() == L'.' || r.back() == L' ')) r.pop_back();
    if (r.size() > 120) r.resize(120);
    static const wchar_t* reserved[] = {L"CON", L"PRN", L"AUX", L"NUL", L"COM1", L"COM2", L"COM3", L"LPT1", L"LPT2"};
    for (auto rv : reserved)
        if (_wcsicmp(r.c_str(), rv) == 0) r = L"_" + r;
    if (r.empty()) r = L"untitled";
    return r;
}

std::wstring QuoteArg(const std::wstring& a) {
    if (!a.empty() && a.find_first_of(L" \t\n\v\"") == std::wstring::npos) return a;
    std::wstring r = L"\"";
    for (size_t i = 0;; ++i) {
        size_t bs = 0;
        while (i < a.size() && a[i] == L'\\') { ++i; ++bs; }
        if (i == a.size()) { r.append(bs * 2, L'\\'); break; }
        if (a[i] == L'"') { r.append(bs * 2 + 1, L'\\'); r += L'"'; }
        else { r.append(bs, L'\\'); r += a[i]; }
    }
    r += L'"';
    return r;
}

std::wstring FindOnPath(const wchar_t* exe) {
    wchar_t buf[MAX_PATH * 2];
    DWORD n = SearchPathW(nullptr, exe, nullptr, (DWORD)(sizeof buf / sizeof buf[0]), buf, nullptr);
    if (n == 0 || n >= sizeof buf / sizeof buf[0]) return L"";
    return buf;
}

bool ReadFileBytes(const std::wstring& path, std::string& out) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER sz;
    GetFileSizeEx(f, &sz);
    out.resize((size_t)sz.QuadPart);
    DWORD got = 0;
    bool ok = sz.QuadPart == 0 || (ReadFile(f, &out[0], (DWORD)out.size(), &got, nullptr) && got == out.size());
    CloseHandle(f);
    return ok;
}

bool WriteFileBytes(const std::wstring& path, const std::string& data) {
    std::wstring tmp = path + L".tmp";
    HANDLE f = CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    DWORD put = 0;
    bool ok = data.empty() || (WriteFile(f, data.data(), (DWORD)data.size(), &put, nullptr) && put == data.size());
    CloseHandle(f);
    if (!ok) { DeleteFileW(tmp.c_str()); return false; }
    return MoveFileExW(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
}

// ============================================================ shell

bool IsProcessElevated() {
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return false;
    TOKEN_ELEVATION te{};
    DWORD size = sizeof te;
    bool elevated = GetTokenInformation(token, TokenElevation, &te, sizeof te, &size) && te.TokenIsElevated;
    CloseHandle(token);
    return elevated;
}

// Handing the path to explorer.exe makes the shell open it under the logged-on
// user. Without this, an elevated process cannot start Store apps such as the
// built-in media player, and nothing happens when the user clicks Play.
static bool OpenViaExplorer(const std::wstring& args) {
    return (INT_PTR)ShellExecuteW(nullptr, nullptr, L"explorer.exe", args.c_str(), nullptr, SW_SHOWNORMAL) > 32;
}

bool ShellOpen(const std::wstring& target) {
    if (!IsProcessElevated() && (INT_PTR)ShellExecuteW(nullptr, L"open", target.c_str(), nullptr, nullptr, SW_SHOWNORMAL) > 32)
        return true;
    return OpenViaExplorer(QuoteArg(target));
}

void ShowInFolder(const std::wstring& path) {
    if (FileExists(path) && !IsProcessElevated()) {
        PIDLIST_ABSOLUTE pidl = nullptr;
        if (SUCCEEDED(SHParseDisplayName(path.c_str(), nullptr, &pidl, 0, nullptr)) && pidl) {
            HRESULT hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
            CoTaskMemFree(pidl);
            if (SUCCEEDED(hr)) return;
        }
    }
    if (FileExists(path) && OpenViaExplorer(L"/select," + QuoteArg(path))) return;
    std::wstring dir = DirExists(path) ? path : PathDir(path);
    if (!dir.empty()) OpenViaExplorer(QuoteArg(dir));
}

bool SetClipboardText(HWND owner, const std::wstring& text) {
    if (!OpenClipboard(owner)) return false;
    EmptyClipboard();
    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, bytes);
    bool ok = false;
    if (h) {
        memcpy(GlobalLock(h), text.c_str(), bytes);
        GlobalUnlock(h);
        ok = SetClipboardData(CF_UNICODETEXT, h) != nullptr;
        if (!ok) GlobalFree(h);
    }
    CloseClipboard();
    return ok;
}

std::wstring GetClipboardText(HWND owner) {
    std::wstring r;
    for (int attempt = 0; attempt < 5; ++attempt) {
        if (OpenClipboard(owner)) break;
        if (attempt == 4) return r;
        Sleep(30);
    }
    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if (h) {
        const wchar_t* p = (const wchar_t*)GlobalLock(h);
        if (p) r = p;
        GlobalUnlock(h);
    }
    CloseClipboard();
    return r;
}

static bool PickWithDialog(HWND owner, std::wstring& path, const wchar_t* title, bool folder,
                           const wchar_t* filterName, const wchar_t* filterSpec) {
    IFileOpenDialog* dlg = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dlg)))) return false;
    DWORD opts = 0;
    dlg->GetOptions(&opts);
    opts |= FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR;
    if (folder) opts |= FOS_PICKFOLDERS;
    else opts |= FOS_FILEMUSTEXIST;
    dlg->SetOptions(opts);
    if (title) dlg->SetTitle(title);
    if (!folder && filterName && filterSpec) {
        COMDLG_FILTERSPEC spec[2] = {{filterName, filterSpec}, {L"*.*", L"*.*"}};
        dlg->SetFileTypes(2, spec);
    }
    std::wstring start = folder ? path : PathDir(path);
    if (!start.empty() && DirExists(start)) {
        IShellItem* si = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(start.c_str(), nullptr, IID_PPV_ARGS(&si)))) {
            dlg->SetFolder(si);
            si->Release();
        }
    }
    bool ok = false;
    if (SUCCEEDED(dlg->Show(owner))) {
        IShellItem* res = nullptr;
        if (SUCCEEDED(dlg->GetResult(&res))) {
            PWSTR p = nullptr;
            if (SUCCEEDED(res->GetDisplayName(SIGDN_FILESYSPATH, &p)) && p) {
                path = p;
                CoTaskMemFree(p);
                ok = true;
            }
            res->Release();
        }
    }
    dlg->Release();
    return ok;
}

bool PickFolder(HWND owner, std::wstring& path, const wchar_t* title) {
    return PickWithDialog(owner, path, title, true, nullptr, nullptr);
}

bool PickFile(HWND owner, std::wstring& path, const wchar_t* title, const wchar_t* filterName, const wchar_t* filterSpec) {
    return PickWithDialog(owner, path, title, false, filterName, filterSpec);
}

bool RecycleFiles(const std::vector<std::wstring>& files) {
    std::wstring list;
    for (auto& f : files) {
        if (!FileExists(f)) continue;
        list += f;
        list += L'\0';
    }
    if (list.empty()) return true;
    list += L'\0';
    SHFILEOPSTRUCTW op{};
    op.wFunc = FO_DELETE;
    op.pFrom = list.c_str();
    op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
    return SHFileOperationW(&op) == 0;
}

// ============================================================ network

int CompareVersions(const std::wstring& a, const std::wstring& b) {
    auto parts = [](const std::wstring& s) {
        std::vector<int> v;
        for (auto& p : Split(s, L'.')) v.push_back(_wtoi(p.c_str()));
        while (v.size() < 4) v.push_back(0);
        return v;
    };
    auto va = parts(a), vb = parts(b);
    for (size_t i = 0; i < 4; ++i)
        if (va[i] != vb[i]) return va[i] < vb[i] ? -1 : 1;
    return 0;
}

namespace {
struct Inet {
    HINTERNET h = nullptr;
    ~Inet() { if (h) WinHttpCloseHandle(h); }
};
}  // namespace

// Fetches a small document. Used for the release check, so the timeouts are
// short: an update check must never leave the app hanging.
bool HttpGetText(const std::wstring& url, std::string& out) {
    out.clear();
    wchar_t host[256] = {}, path[2048] = {};
    URL_COMPONENTS uc{};
    uc.dwStructSize = sizeof uc;
    uc.lpszHostName = host;
    uc.dwHostNameLength = (DWORD)(sizeof host / sizeof host[0]) - 1;
    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = (DWORD)(sizeof path / sizeof path[0]) - 1;
    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &uc)) return false;

    Inet ses, con, req;
    ses.h = WinHttpOpen(L"YouTubeToPC/" APP_VERSION, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
                        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!ses.h) return false;
    WinHttpSetTimeouts(ses.h, 8000, 8000, 12000, 12000);
    con.h = WinHttpConnect(ses.h, host, uc.nPort, 0);
    if (!con.h) return false;
    DWORD flags = uc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0;
    req.h = WinHttpOpenRequest(con.h, L"GET", path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!req.h) return false;
    const wchar_t* headers = L"Accept: application/vnd.github+json\r\nX-GitHub-Api-Version: 2022-11-28\r\n";
    if (!WinHttpSendRequest(req.h, headers, (DWORD)-1, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) return false;
    if (!WinHttpReceiveResponse(req.h, nullptr)) return false;

    DWORD status = 0, size = sizeof status;
    WinHttpQueryHeaders(req.h, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
                        &status, &size, WINHTTP_NO_HEADER_INDEX);
    if (status != 200) return false;

    for (;;) {
        DWORD avail = 0;
        if (!WinHttpQueryDataAvailable(req.h, &avail) || avail == 0) break;
        size_t had = out.size();
        out.resize(had + avail);
        DWORD got = 0;
        if (!WinHttpReadData(req.h, &out[had], avail, &got)) return false;
        out.resize(had + got);
        if (out.size() > 4u * 1024 * 1024) break;  // the release JSON is tiny
    }
    return !out.empty();
}

namespace {
// Reports download progress through urlmon and lets the caller abort.
class DlProgress : public IBindStatusCallback {
public:
    using Fn = std::function<bool(long long done, long long total)>;
    explicit DlProgress(Fn fn) : fn_(std::move(fn)) {}
    STDMETHODIMP QueryInterface(REFIID riid, void** out) override {
        if (riid == IID_IUnknown || riid == IID_IBindStatusCallback) {
            *out = static_cast<IBindStatusCallback*>(this);
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return 2; }
    STDMETHODIMP_(ULONG) Release() override { return 1; }
    STDMETHODIMP OnStartBinding(DWORD, IBinding*) override { return S_OK; }
    STDMETHODIMP GetPriority(LONG*) override { return E_NOTIMPL; }
    STDMETHODIMP OnLowResource(DWORD) override { return S_OK; }
    STDMETHODIMP OnProgress(ULONG now, ULONG max, ULONG, LPCWSTR) override {
        return (fn_ && !fn_(now, max)) ? E_ABORT : S_OK;
    }
    STDMETHODIMP OnStopBinding(HRESULT, LPCWSTR) override { return S_OK; }
    STDMETHODIMP GetBindInfo(DWORD*, BINDINFO*) override { return S_OK; }
    STDMETHODIMP OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*) override { return S_OK; }
    STDMETHODIMP OnObjectAvailable(REFIID, IUnknown*) override { return S_OK; }

private:
    Fn fn_;
};
}  // namespace

// Writes to a .part file first so a half-finished download can never be mistaken
// for a usable one.
bool DownloadToFile(const std::wstring& url, const std::wstring& dest,
                    const std::function<bool(long long done, long long total)>& progress) {
    DlProgress cb(progress);
    std::wstring tmp = dest + L".part";
    DeleteFileW(tmp.c_str());
    if (SUCCEEDED(URLDownloadToFileW(nullptr, url.c_str(), tmp.c_str(), 0, &cb)) && FileSize(tmp) > 0) {
        DeleteFileW(dest.c_str());
        if (MoveFileW(tmp.c_str(), dest.c_str())) return true;
    }
    DeleteFileW(tmp.c_str());
    return false;
}

std::wstring GetText(HWND h) {
    int n = GetWindowTextLengthW(h);
    std::wstring s(n + 1, L'\0');
    GetWindowTextW(h, &s[0], n + 1);
    s.resize(n);
    return s;
}

// ============================================================ child processes

namespace {
struct ProcSlot {
    HANDLE job = nullptr;
    HANDLE process = nullptr;
    bool canceled = false;
};
std::mutex g_procMx;
std::map<int, ProcSlot> g_procs;
std::atomic<int> g_procNext{0};

void ProcReader(int id, HANDLE rd, HANDLE proc, HWND notify, bool lineMode) {
    std::string buf, all;
    std::vector<char> tmp(1 << 16);
    DWORD n = 0;
    auto postLine = [&](const std::string& raw) {
        std::string line = raw;
        while (!line.empty() && (line.back() == ' ' || line.back() == '\r')) line.pop_back();
        if (line.empty()) return;
        auto* pl = new ProcLine{id, W(line)};
        if (!PostMessageW(notify, WM_APP_PROC_LINE, 0, (LPARAM)pl)) delete pl;
    };
    while (ReadFile(rd, tmp.data(), (DWORD)tmp.size(), &n, nullptr) && n > 0) {
        if (lineMode) {
            buf.append(tmp.data(), n);
            size_t start = 0, pos;
            while ((pos = buf.find_first_of("\r\n", start)) != std::string::npos) {
                postLine(buf.substr(start, pos - start));
                start = pos + 1;
            }
            buf.erase(0, start);
        } else {
            all.append(tmp.data(), n);
        }
    }
    if (lineMode && !buf.empty()) postLine(buf);
    CloseHandle(rd);
    WaitForSingleObject(proc, INFINITE);
    DWORD code = 1;
    GetExitCodeProcess(proc, &code);
    bool canceled = false;
    {
        std::lock_guard<std::mutex> lk(g_procMx);
        auto it = g_procs.find(id);
        if (it != g_procs.end()) {
            canceled = it->second.canceled;
            if (it->second.job) CloseHandle(it->second.job);
            g_procs.erase(it);
        }
    }
    CloseHandle(proc);
    auto* pe = new ProcExit{id, code, canceled, std::move(all)};
    if (!PostMessageW(notify, WM_APP_PROC_EXIT, 0, (LPARAM)pe)) delete pe;
}
}  // namespace

int ProcStart(const std::wstring& exe, const std::wstring& args, HWND notify, bool lineMode) {
    if (exe.empty()) return 0;
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE rd = nullptr, wr = nullptr;
    if (!CreatePipe(&rd, &wr, &sa, 0)) return 0;
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);
    HANDLE nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, nullptr);

    SIZE_T attrSize = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &attrSize);
    std::vector<BYTE> attrBuf(attrSize);
    auto attrs = (LPPROC_THREAD_ATTRIBUTE_LIST)attrBuf.data();
    InitializeProcThreadAttributeList(attrs, 1, 0, &attrSize);
    HANDLE inherit[2] = {wr, nul};
    UpdateProcThreadAttribute(attrs, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherit, sizeof inherit, nullptr, nullptr);

    STARTUPINFOEXW si{};
    si.StartupInfo.cb = sizeof si;
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.StartupInfo.wShowWindow = SW_HIDE;
    si.StartupInfo.hStdInput = nul;
    si.StartupInfo.hStdOutput = wr;
    si.StartupInfo.hStdError = wr;
    si.lpAttributeList = attrs;

    std::wstring cmd = QuoteArg(exe) + L" " + args;
    std::wstring workDir = PathDir(exe);
    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessW(exe.c_str(), &cmd[0], nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW | CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT,
                             nullptr, workDir.empty() ? nullptr : workDir.c_str(), &si.StartupInfo, &pi);
    DeleteProcThreadAttributeList(attrs);
    CloseHandle(wr);
    if (nul != INVALID_HANDLE_VALUE) CloseHandle(nul);
    if (!ok) {
        CloseHandle(rd);
        return 0;
    }
    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (job) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION li{};
        li.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(job, JobObjectExtendedLimitInformation, &li, sizeof li);
        AssignProcessToJobObject(job, pi.hProcess);
    }
    int id = ++g_procNext;
    {
        std::lock_guard<std::mutex> lk(g_procMx);
        g_procs[id] = ProcSlot{job, pi.hProcess, false};
    }
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    std::thread(ProcReader, id, rd, pi.hProcess, notify, lineMode).detach();
    return id;
}

void ProcCancel(int id) {
    std::lock_guard<std::mutex> lk(g_procMx);
    auto it = g_procs.find(id);
    if (it == g_procs.end()) return;
    it->second.canceled = true;
    if (it->second.job) TerminateJobObject(it->second.job, 1);
    else TerminateProcess(it->second.process, 1);
}

void ProcCancelAll() {
    std::lock_guard<std::mutex> lk(g_procMx);
    for (auto& kv : g_procs) {
        kv.second.canceled = true;
        if (kv.second.job) TerminateJobObject(kv.second.job, 1);
        else TerminateProcess(kv.second.process, 1);
    }
}

// ============================================================ thumbnails

namespace {
struct ThumbJob {
    std::wstring vid;
    HWND hwnd;
    WPARAM tag;
    int w, h;
};
std::mutex g_thMx;
std::condition_variable g_thCv;
std::deque<ThumbJob> g_thQueue;

bool ValidVideoId(const std::wstring& id) {
    if (id.size() < 6 || id.size() > 20) return false;
    for (wchar_t c : id)
        if (!(iswalnum(c) || c == L'-' || c == L'_')) return false;
    return true;
}

HBITMAP LoadScaled(const std::wstring& path, int w, int h) {
    Gdiplus::Bitmap src(path.c_str());
    if (src.GetLastStatus() != Gdiplus::Ok || src.GetWidth() == 0) return nullptr;
    Gdiplus::Bitmap dst(w, h, PixelFormat32bppARGB);
    {
        Gdiplus::Graphics g(&dst);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        double sw = src.GetWidth(), sh = src.GetHeight();
        double scale = std::max(w / sw, h / sh);
        double cw = w / scale, ch = h / scale;
        Gdiplus::ImageAttributes ia;
        ia.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
        g.DrawImage(&src, Gdiplus::Rect(0, 0, w, h), (INT)((sw - cw) / 2), (INT)((sh - ch) / 2), (INT)cw, (INT)ch,
                    Gdiplus::UnitPixel, &ia);
    }
    HBITMAP hb = nullptr;
    dst.GetHBITMAP(Gdiplus::Color(255, 255, 255), &hb);
    return hb;
}

void ThumbWorker() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    std::wstring dir = PathJoin(LocalDataDir(), L"thumbs");
    EnsureDir(dir);
    for (;;) {
        ThumbJob job;
        {
            std::unique_lock<std::mutex> lk(g_thMx);
            g_thCv.wait(lk, [] { return !g_thQueue.empty(); });
            job = g_thQueue.front();
            g_thQueue.pop_front();
        }
        HBITMAP hb = nullptr;
        if (ValidVideoId(job.vid) && IsWindow(job.hwnd)) {
            std::wstring file = PathJoin(dir, job.vid + L".jpg");
            if (!FileExists(file)) {
                std::wstring url = L"https://i.ytimg.com/vi/" + job.vid + L"/mqdefault.jpg";
                std::wstring tmp = file + L".part";
                if (SUCCEEDED(URLDownloadToFileW(nullptr, url.c_str(), tmp.c_str(), 0, nullptr)))
                    MoveFileExW(tmp.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING);
                else
                    DeleteFileW(tmp.c_str());
            }
            if (FileExists(file)) hb = LoadScaled(file, job.w, job.h);
        }
        if (!PostMessageW(job.hwnd, WM_APP_THUMB, job.tag, (LPARAM)hb) && hb) DeleteObject(hb);
    }
}
}  // namespace

void ThumbStart() {
    for (int i = 0; i < 3; ++i) std::thread(ThumbWorker).detach();
}

void ThumbRequest(const std::wstring& videoId, HWND notify, WPARAM tag, int w, int h) {
    {
        std::lock_guard<std::mutex> lk(g_thMx);
        g_thQueue.push_back(ThumbJob{videoId, notify, tag, w, h});
    }
    g_thCv.notify_one();
}
