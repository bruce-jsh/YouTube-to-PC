// YouTube to PC installer and uninstaller.
// Setup.exe = this stub + compressed payload (see src/tools/pack.cpp).
// Uninstall.exe = this stub without payload.
#include "common.h"
#include <compressapi.h>
#include <urlmon.h>
#include <wininet.h>
#include <cstdint>
#include <thread>
#include <functional>

namespace {

#define UNINSTALL_SUBKEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\YouTubeToPC"
#define APP_EXE L"YouTubeToPC.exe"
#define SHORTCUT_NAME L"YouTube to PC.lnk"

#pragma pack(push, 1)
struct PackFooter {
    char magic[8];
    uint64_t payloadOffset;
    uint64_t totalRaw;
    uint32_t fileCount;
    uint32_t reserved;
};
#pragma pack(pop)
const char kMagic[8] = {'Y', 'T', '2', 'P', 'C', 'P', 'K', '1'};

enum { IDC_LANG = 100, IDC_DIR, IDC_BROWSE, IDC_DESKTOP, IDC_LAUNCH };
enum { WM_SETUP_PROGRESS = WM_APP + 20, WM_SETUP_DONE, WM_SETUP_STATUS };

// Tools are fetched at install time: the app needs the internet anyway, and the
// yt-dlp build must be current or YouTube refuses the download.
#define URL_YTDLP L"https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe"
#define URL_QJS L"https://github.com/quickjs-ng/quickjs/releases/download/v0.17.0/qjs-windows-x86_64.exe"
#define URL_FFMPEG L"https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-master-latest-win64-gpl-shared.zip"
enum { RESULT_RESTART = 100 };  // language changed: rebuild the dialog

constexpr int DW = 580, DH = 402, HEADER = 100, FOOT = 342;

struct State {
    std::wstring self;
    PackFooter footer{};
    HWND dlg = nullptr, cbLang = nullptr, edDir = nullptr, btnBrowse = nullptr, chkDesktop = nullptr, chkLaunch = nullptr,
         btnOk = nullptr, btnCancel = nullptr, status = nullptr, lblLoc = nullptr, lblSpace = nullptr;
    HICON icon = nullptr;
    int progress = -1;
    bool installing = false, finished = false;
    bool desktop = true, launch = true;
    std::wstring dir, presetDir, installedVer;
} g;

// Starts the app as the logged-on user. An app started by an elevated installer
// would inherit administrator rights, and then Explorer integration and Store
// media players stop working.
bool RunAsShellUser(const std::wstring& exe, const std::wstring& workDir) {
    HWND shell = GetShellWindow();
    if (!shell) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(shell, &pid);
    if (!pid) return false;
    HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!proc) return false;
    HANDLE token = nullptr, dup = nullptr;
    bool ok = false;
    if (OpenProcessToken(proc, TOKEN_DUPLICATE, &token)) {
        if (DuplicateTokenEx(token, TOKEN_ALL_ACCESS, nullptr, SecurityImpersonation, TokenPrimary, &dup)) {
            STARTUPINFOW si{sizeof si};
            PROCESS_INFORMATION pi{};
            std::wstring cmd = QuoteArg(exe);
            ok = CreateProcessWithTokenW(dup, 0, exe.c_str(), &cmd[0], 0, nullptr, workDir.c_str(), &si, &pi) != 0;
            if (ok) {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
            }
            CloseHandle(dup);
        }
        CloseHandle(token);
    }
    CloseHandle(proc);
    return ok;
}

void LaunchApp(const std::wstring& dir) {
    std::wstring exe = PathJoin(dir, APP_EXE);
    if (IsProcessElevated() && RunAsShellUser(exe, dir)) return;
    ShellExecuteW(nullptr, L"open", exe.c_str(), nullptr, dir.c_str(), SW_SHOWNORMAL);
}

bool ReadFooter(const std::wstring& path, PackFooter& f) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{}, pos{};
    GetFileSizeEx(h, &size);
    bool ok = false;
    if (size.QuadPart > (LONGLONG)sizeof f) {
        pos.QuadPart = size.QuadPart - sizeof f;
        DWORD got = 0;
        if (SetFilePointerEx(h, pos, nullptr, FILE_BEGIN) && ReadFile(h, &f, sizeof f, &got, nullptr) && got == sizeof f)
            ok = memcmp(f.magic, kMagic, 8) == 0 && f.payloadOffset < (uint64_t)size.QuadPart;
    }
    CloseHandle(h);
    return ok;
}

std::wstring RegString(HKEY root, const wchar_t* name) {
    wchar_t buf[MAX_PATH * 2];
    DWORD size = sizeof buf;
    if (RegGetValueW(root, UNINSTALL_SUBKEY, name, RRF_RT_REG_SZ, nullptr, buf, &size) == ERROR_SUCCESS) return buf;
    return L"";
}

std::wstring DefaultInstallDir() {
    for (HKEY root : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER}) {
        std::wstring prev = RegString(root, L"InstallLocation");
        if (!prev.empty()) return prev;
    }
    std::wstring base = KnownFolder(FOLDERID_ProgramFiles);
    if (base.empty()) base = L"C:\\Program Files";
    return PathJoin(base, APP_FOLDER);
}

// Version of an existing installation, empty when this is a fresh install.
std::wstring InstalledVersion() {
    for (HKEY root : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER}) {
        std::wstring dir = RegString(root, L"InstallLocation");
        if (dir.empty() || !FileExists(PathJoin(dir, APP_EXE))) continue;
        std::wstring v = RegString(root, L"DisplayVersion");
        return v.empty() ? L"?" : v;
    }
    return L"";
}

// Compares dotted version numbers: <0, 0 or >0.
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

std::vector<std::wstring> ReadInstallLog(const std::wstring& dir) {
    std::vector<std::wstring> files;
    std::string log;
    if (!ReadFileBytes(PathJoin(dir, L"install.log"), log)) return files;
    for (auto& line : Split(W(log), L'\n')) {
        std::wstring f = Trim(line);
        if (!f.empty() && f.find(L"..") == std::wstring::npos && f.find(L':') == std::wstring::npos) files.push_back(f);
    }
    return files;
}

// Asks a running YouTube to PC to quit. Returns false if it is still running.
bool QuitRunningApp(HWND owner, bool ask) {
    HWND w = FindWindowW(MAIN_CLASS, nullptr);
    if (!w) return true;
    if (ask && MessageBoxW(owner, T(S_SU_RUNNING), APP_NAME, MB_YESNO | MB_ICONQUESTION) != IDYES) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    HANDLE proc = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, pid);
    PostMessageW(w, RegisterWindowMessageW(L"YouTubeToPC.Quit"), 0, 0);
    bool gone = true;
    if (proc) {
        if (WaitForSingleObject(proc, 8000) != WAIT_OBJECT_0) {
            TerminateProcess(proc, 0);
            gone = WaitForSingleObject(proc, 3000) == WAIT_OBJECT_0;
        }
        CloseHandle(proc);
    }
    return gone;
}

bool MakeShortcut(const std::wstring& lnk, const std::wstring& target, const std::wstring& workDir) {
    IShellLinkW* sl = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&sl)))) return false;
    sl->SetPath(target.c_str());
    sl->SetWorkingDirectory(workDir.c_str());
    sl->SetDescription(APP_NAME);
    sl->SetIconLocation(target.c_str(), 0);
    bool ok = false;
    IPersistFile* pf = nullptr;
    if (SUCCEEDED(sl->QueryInterface(IID_PPV_ARGS(&pf)))) {
        ok = SUCCEEDED(pf->Save(lnk.c_str(), TRUE));
        pf->Release();
    }
    sl->Release();
    return ok;
}

// ---------------------------------------------------------------- downloading

void DeleteTree(const std::wstring& dir);

void PostStatus(const std::wstring& text) {
    PostMessageW(g.dlg, WM_SETUP_STATUS, 0, (LPARAM) new std::wstring(text));
}

// Reports download progress through urlmon so the installer can show a bar.
class DlProgress : public IBindStatusCallback {
public:
    using Fn = std::function<void(uint64_t done, uint64_t total)>;
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
        if (fn_) fn_(now, max);
        return S_OK;
    }
    STDMETHODIMP OnStopBinding(HRESULT, LPCWSTR) override { return S_OK; }
    STDMETHODIMP GetBindInfo(DWORD*, BINDINFO*) override { return S_OK; }
    STDMETHODIMP OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*) override { return S_OK; }
    STDMETHODIMP OnObjectAvailable(REFIID, IUnknown*) override { return S_OK; }

private:
    Fn fn_;
};

// Downloads one file, retrying a few times so a brief network hiccup is not fatal.
bool Download(const std::wstring& url, const std::wstring& dest, const std::wstring& label,
              int basePermille, int spanPermille) {
    constexpr int kTries = 3;
    for (int attempt = 1; attempt <= kTries; ++attempt) {
        if (attempt > 1) {
            PostStatus(Fmt(T(S_SU_RETRYING), attempt, kTries));
            Sleep(1500);
        }
        DeleteUrlCacheEntryW(url.c_str());  // always fetch a fresh copy
        DlProgress cb([&](uint64_t done, uint64_t total) {
            PostStatus(Fmt(T(S_SU_DOWNLOADING), label.c_str(), FormatSize((long long)done).c_str(),
                           total ? FormatSize((long long)total).c_str() : L"?"));
            if (total)
                PostMessageW(g.dlg, WM_SETUP_PROGRESS, basePermille + (int)(spanPermille * done / total), 0);
        });
        std::wstring tmp = dest + L".part";
        DeleteFileW(tmp.c_str());
        if (SUCCEEDED(URLDownloadToFileW(nullptr, url.c_str(), tmp.c_str(), 0, &cb)) && FileSize(tmp) > 0) {
            DeleteFileW(dest.c_str());
            if (MoveFileW(tmp.c_str(), dest.c_str())) return true;
        }
        DeleteFileW(tmp.c_str());
    }
    return false;
}

// tar.exe ships with Windows 10 1803 and later and handles zip archives.
bool ExtractZip(const std::wstring& zip, const std::wstring& destDir) {
    EnsureDir(destDir);
    wchar_t sys[MAX_PATH];
    GetSystemDirectoryW(sys, MAX_PATH);
    std::wstring exe = std::wstring(sys) + L"\\tar.exe";
    if (!FileExists(exe)) return false;
    std::wstring cmd = QuoteArg(exe) + L" -xf " + QuoteArg(zip) + L" -C " + QuoteArg(destDir);
    STARTUPINFOW si{sizeof si};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(exe.c_str(), &cmd[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, destDir.c_str(), &si, &pi))
        return false;
    WaitForSingleObject(pi.hProcess, 10 * 60 * 1000);
    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return code == 0;
}

// Finds <root>\*\bin\ffmpeg.exe produced by the FFmpeg archive.
std::wstring FindExtractedFfmpegBin(const std::wstring& root) {
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((root + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return L"";
    std::wstring found;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        std::wstring name = fd.cFileName;
        if (name == L"." || name == L"..") continue;
        std::wstring bin = root + L"\\" + name + L"\\bin";
        if (FileExists(bin + L"\\ffmpeg.exe")) {
            found = bin;
            break;
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    return found;
}

bool ReadExact(HANDLE h, void* p, DWORD n) {
    DWORD got = 0;
    return ReadFile(h, p, n, &got, nullptr) && got == n;
}

bool WriteExact(HANDLE h, const void* p, DWORD n) {
    DWORD put = 0;
    return WriteFile(h, p, n, &put, nullptr) && put == n;
}

void PostDone(const std::wstring& error) {
    PostMessageW(g.dlg, WM_SETUP_DONE, error.empty() ? 1 : 0, error.empty() ? 0 : (LPARAM) new std::wstring(error));
}

// Files left over from an older version that the new one no longer ships.
void RemoveStaleFiles(const std::wstring& dir, const std::vector<std::wstring>& oldFiles,
                      const std::vector<std::wstring>& newFiles) {
    std::set<std::wstring> keep;
    for (auto& f : newFiles) keep.insert(Lower(f));
    std::set<std::wstring> dirs;
    for (auto& f : oldFiles) {
        if (keep.count(Lower(f))) continue;
        DeleteFileW(PathJoin(dir, f).c_str());
        for (std::wstring p = PathDir(f); !p.empty(); p = PathDir(p)) dirs.insert(p);
    }
    std::vector<std::wstring> sorted(dirs.begin(), dirs.end());
    std::sort(sorted.begin(), sorted.end(), [](const std::wstring& a, const std::wstring& b) { return a.size() > b.size(); });
    for (auto& d : sorted) RemoveDirectoryW(PathJoin(dir, d).c_str());  // only removes empty folders
}

// Everything the app needs besides its own executable.
struct ToolPlan {
    std::wstring ytdlpSrc, qjsSrc, ffmpegBin;  // staged in a temporary folder
    std::wstring temp;
};

// Downloads every tool into a temporary folder first, so a failed download never
// leaves a half-installed program behind. The app keeps its own copies rather
// than relying on whatever happens to be on the PC: those can be moved or
// deleted at any time, which would quietly break downloading.
bool PrepareTools(ToolPlan& plan, std::wstring& error) {
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    plan.temp = PathJoin(tmp, L"YouTubeToPC-setup");
    DeleteTree(plan.temp);
    EnsureDir(plan.temp);

    PostStatus(T(S_SU_PREPARING));
    PostMessageW(g.dlg, WM_SETUP_PROGRESS, 10, 0);

    // yt-dlp: 20-250 permille
    plan.ytdlpSrc = PathJoin(plan.temp, L"yt-dlp.exe");
    if (!Download(URL_YTDLP, plan.ytdlpSrc, L"yt-dlp", 20, 230)) {
        error = Fmt(T(S_SU_DL_FAILED), L"yt-dlp");
        return false;
    }
    // QuickJS: 250-300 permille
    plan.qjsSrc = PathJoin(plan.temp, L"qjs.exe");
    if (!Download(URL_QJS, plan.qjsSrc, L"QuickJS", 250, 50)) {
        error = Fmt(T(S_SU_DL_FAILED), L"QuickJS");
        return false;
    }
    // FFmpeg: 300-800 permille
    std::wstring zip = PathJoin(plan.temp, L"ffmpeg.zip");
    if (!Download(URL_FFMPEG, zip, L"FFmpeg", 300, 420)) {
        error = Fmt(T(S_SU_DL_FAILED), L"FFmpeg");
        return false;
    }
    PostStatus(Fmt(T(S_SU_EXTRACTING), L"FFmpeg"));
    PostMessageW(g.dlg, WM_SETUP_PROGRESS, 730, 0);
    std::wstring out = PathJoin(plan.temp, L"ffmpeg");
    if (!ExtractZip(zip, out) || (plan.ffmpegBin = FindExtractedFfmpegBin(out)).empty()) {
        error = Fmt(T(S_SU_EXTRACTING), L"FFmpeg");
        return false;
    }
    DeleteFileW(zip.c_str());
    PostMessageW(g.dlg, WM_SETUP_PROGRESS, 800, 0);
    return true;
}

// Copies the staged tools into the install folder and records what was written.
bool InstallTools(const ToolPlan& plan, const std::wstring& dir, std::vector<std::wstring>& installed, std::wstring& error) {
    std::wstring tools = PathJoin(dir, L"tools");
    EnsureDir(tools);
    auto put = [&](const std::wstring& src, const std::wstring& rel) {
        if (src.empty()) return true;
        std::wstring dst = PathJoin(dir, rel);
        EnsureDir(PathDir(dst));
        if (!CopyFileW(src.c_str(), dst.c_str(), FALSE)) {
            error = Fmt(T(S_SU_ERR_WRITE), dst.c_str());
            return false;
        }
        installed.push_back(rel);
        return true;
    };
    if (!put(plan.ytdlpSrc, L"tools\\yt-dlp.exe")) return false;
    if (!put(plan.qjsSrc, L"tools\\qjs.exe")) return false;
    if (!plan.ffmpegBin.empty()) {
        // FFmpeg is GPL, so ship the licence that came with the build we installed.
        std::wstring lic = PathJoin(PathDir(plan.ffmpegBin), L"LICENSE.txt");
        if (FileExists(lic) && !put(lic, L"licenses\\ffmpeg-LICENSE.txt")) return false;
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW((plan.ffmpegBin + L"\\*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) {
            error = T(S_SU_ERR_DATA);
            return false;
        }
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            std::wstring name = fd.cFileName;
            if (_wcsicmp(name.c_str(), L"ffplay.exe") == 0) continue;  // not needed
            if (!put(plan.ffmpegBin + L"\\" + name, L"tools\\ffmpeg\\" + name)) {
                FindClose(h);
                return false;
            }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }
    return true;
}

void InstallWorker(std::wstring dir, bool desktop, int lang) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    std::vector<std::wstring> oldFiles = ReadInstallLog(dir);

    ToolPlan plan;
    std::wstring toolError;
    if (!PrepareTools(plan, toolError)) {
        DeleteTree(plan.temp);
        CoUninitialize();
        return PostDone(toolError);
    }
    PostStatus(T(S_SU_INSTALLING));
    HANDLE src = CreateFileW(g.self.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (src == INVALID_HANDLE_VALUE) return PostDone(T(S_SU_ERR_DATA));
    DECOMPRESSOR_HANDLE dec = nullptr;
    if (!CreateDecompressor(COMPRESS_ALGORITHM_LZMS, nullptr, &dec)) {
        CloseHandle(src);
        return PostDone(T(S_SU_ERR_DATA));
    }
    std::wstring error;
    std::vector<std::wstring> installed;
    LARGE_INTEGER pos;
    pos.QuadPart = (LONGLONG)g.footer.payloadOffset;
    SetFilePointerEx(src, pos, nullptr, FILE_BEGIN);
    EnsureDir(dir);
    uint64_t doneRaw = 0;
    int lastPermille = -1;
    std::vector<BYTE> stored, raw;
    for (uint32_t i = 0; i < g.footer.fileCount && error.empty(); ++i) {
        uint16_t len = 0;
        if (!ReadExact(src, &len, sizeof len) || len == 0 || len > 1024) { error = T(S_SU_ERR_DATA); break; }
        std::wstring rel(len, L'\0');
        uint64_t size = 0;
        uint32_t blocks = 0;
        if (!ReadExact(src, &rel[0], len * sizeof(wchar_t)) || !ReadExact(src, &size, sizeof size) ||
            !ReadExact(src, &blocks, sizeof blocks) || rel.find(L"..") != std::wstring::npos ||
            rel.find(L':') != std::wstring::npos || rel[0] == L'\\') {
            error = T(S_SU_ERR_DATA);
            break;
        }
        std::wstring full = PathJoin(dir, rel);
        EnsureDir(PathDir(full));
        HANDLE out = CreateFileW(full.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (out == INVALID_HANDLE_VALUE) {
            error = Fmt(T(S_SU_ERR_WRITE), full.c_str());
            break;
        }
        for (uint32_t b = 0; b < blocks; ++b) {
            uint32_t rawLen = 0, storedLen = 0;
            uint8_t method = 0;
            if (!ReadExact(src, &rawLen, 4) || !ReadExact(src, &storedLen, 4) || !ReadExact(src, &method, 1) ||
                rawLen > (64u << 20) || storedLen > (64u << 20)) {
                error = T(S_SU_ERR_DATA);
                break;
            }
            stored.resize(storedLen);
            if (storedLen && !ReadExact(src, stored.data(), storedLen)) { error = T(S_SU_ERR_DATA); break; }
            const BYTE* data = stored.data();
            if (method == 1) {
                raw.resize(rawLen);
                SIZE_T got = 0;
                if (!Decompress(dec, stored.data(), storedLen, raw.data(), rawLen, &got) || got != rawLen) {
                    error = T(S_SU_ERR_DATA);
                    break;
                }
                data = raw.data();
            }
            if (rawLen && !WriteExact(out, data, rawLen)) {
                error = T(S_SU_ERR_DISK);
                break;
            }
            doneRaw += rawLen;
            int pm = 800 + (g.footer.totalRaw ? (int)(doneRaw * 150 / g.footer.totalRaw) : 0);
            if (pm != lastPermille) {
                lastPermille = pm;
                PostMessageW(g.dlg, WM_SETUP_PROGRESS, pm, 0);
            }
        }
        CloseHandle(out);
        if (error.empty()) installed.push_back(rel);
        else DeleteFileW(full.c_str());
    }
    CloseDecompressor(dec);

    if (error.empty()) {  // Uninstall.exe = the stub part of this file
        std::wstring un = PathJoin(dir, L"Uninstall.exe");
        HANDLE out = CreateFileW(un.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (out == INVALID_HANDLE_VALUE) error = T(S_SU_ERR_UNINST);
        else {
            pos.QuadPart = 0;
            SetFilePointerEx(src, pos, nullptr, FILE_BEGIN);
            std::vector<BYTE> buf(1 << 20);
            uint64_t left = g.footer.payloadOffset;
            while (left > 0 && error.empty()) {
                DWORD n = (DWORD)std::min<uint64_t>(left, buf.size());
                if (!ReadExact(src, buf.data(), n) || !WriteExact(out, buf.data(), n)) error = T(S_SU_ERR_UNINST);
                left -= n;
            }
            CloseHandle(out);
            installed.push_back(L"Uninstall.exe");
        }
    }
    CloseHandle(src);

    if (error.empty()) InstallTools(plan, dir, installed, error);
    DeleteTree(plan.temp);

    if (error.empty()) {
        // Language chosen here becomes the default for users who have not run the app yet.
        // Tool paths are left empty on purpose: the app uses the copies in its own
        // folder, which it fully controls.
        std::wstring def = L"[settings]\r\nlanguage=" + std::wstring(kLangs[lang].code) + L"\r\n";
        WriteFileBytes(PathJoin(dir, L"default.ini"), U8(def));
        installed.push_back(L"default.ini");

        installed.push_back(L"install.log");
        std::wstring log;
        for (auto& f : installed) log += f + L"\r\n";
        WriteFileBytes(PathJoin(dir, L"install.log"), U8(log));
        if (!oldFiles.empty()) RemoveStaleFiles(dir, oldFiles, installed);

        std::wstring exe = PathJoin(dir, APP_EXE);
        // Machine-wide only when installing outside the user's own profile. On an
        // Administrator account every process is elevated, so elevation alone is
        // not a reliable signal.
        std::wstring profile = Lower(KnownFolder(FOLDERID_Profile));
        bool machine = profile.empty() || !StartsWith(Lower(dir), profile);
        HKEY root = machine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
        HKEY k = nullptr;
        if (RegCreateKeyExW(root, UNINSTALL_SUBKEY, 0, nullptr, 0, KEY_WRITE, nullptr, &k, nullptr) != ERROR_SUCCESS && machine) {
            root = HKEY_CURRENT_USER;
            RegCreateKeyExW(root, UNINSTALL_SUBKEY, 0, nullptr, 0, KEY_WRITE, nullptr, &k, nullptr);
        }
        if (k) {
            auto sz = [&](const wchar_t* n, const std::wstring& v) {
                RegSetValueExW(k, n, 0, REG_SZ, (const BYTE*)v.c_str(), (DWORD)((v.size() + 1) * sizeof(wchar_t)));
            };
            auto dw = [&](const wchar_t* n, DWORD v) { RegSetValueExW(k, n, 0, REG_DWORD, (const BYTE*)&v, sizeof v); };
            SYSTEMTIME st;
            GetLocalTime(&st);
            sz(L"DisplayName", APP_NAME);
            sz(L"DisplayVersion", APP_VERSION);
            sz(L"Publisher", L"Bruce Jung");
            sz(L"DisplayIcon", exe + L",0");
            sz(L"InstallLocation", dir);
            sz(L"UninstallString", L"\"" + PathJoin(dir, L"Uninstall.exe") + L"\"");
            sz(L"InstallDate", Fmt(L"%04d%02d%02d", st.wYear, st.wMonth, st.wDay));
            dw(L"NoModify", 1);
            dw(L"NoRepair", 1);
            dw(L"EstimatedSize", (DWORD)(g.footer.totalRaw / 1024));
            RegCloseKey(k);
        }
        // An older version may have registered under the other root or put its
        // shortcuts elsewhere; drop those so only one entry is left behind.
        RegDeleteTreeW(root == HKEY_LOCAL_MACHINE ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, UNINSTALL_SUBKEY);
        std::wstring programs = KnownFolder(machine ? FOLDERID_CommonPrograms : FOLDERID_Programs);
        if (!programs.empty()) MakeShortcut(PathJoin(programs, SHORTCUT_NAME), exe, dir);
        std::wstring deskDir = KnownFolder(machine ? FOLDERID_PublicDesktop : FOLDERID_Desktop);
        std::wstring desk = PathJoin(deskDir, SHORTCUT_NAME);
        if (desktop) MakeShortcut(desk, exe, dir);
        else DeleteFileW(desk.c_str());
        for (REFKNOWNFOLDERID id : {machine ? FOLDERID_Programs : FOLDERID_CommonPrograms,
                                    machine ? FOLDERID_Desktop : FOLDERID_PublicDesktop}) {
            std::wstring f = KnownFolder(id);
            if (!f.empty()) DeleteFileW(PathJoin(f, SHORTCUT_NAME).c_str());
        }
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    }
    CoUninitialize();
    PostDone(error);
}

void SetBusy(bool busy) {
    for (HWND h : {g.cbLang, g.edDir, g.btnBrowse, g.chkDesktop, g.btnOk, g.btnCancel}) EnableWindow(h, !busy);
}

void InvalidateProgress() {
    RECT r{0, S(268), S(DW), S(FOOT)};
    InvalidateRect(g.dlg, &r, TRUE);
}

// Program Files needs administrator rights: restart ourselves elevated.
bool RelaunchElevated(const std::wstring& dir) {
    std::wstring args = L"/dir " + QuoteArg(dir) + L" /lang " + kLangs[g_lang].code +
                        (Button_GetCheck(g.chkDesktop) == BST_CHECKED ? L" /desktop" : L"") +
                        (Button_GetCheck(g.chkLaunch) == BST_CHECKED ? L" /launch" : L"");
    SHELLEXECUTEINFOW ei{sizeof ei};
    ei.fMask = SEE_MASK_NOCLOSEPROCESS;
    ei.lpVerb = L"runas";
    ei.lpFile = g.self.c_str();
    ei.lpParameters = args.c_str();
    ei.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&ei)) return false;
    if (ei.hProcess) CloseHandle(ei.hProcess);
    return true;
}

void StartInstall(bool autoStart) {
    std::wstring dir = Trim(GetText(g.edDir));
    while (!dir.empty() && (dir.back() == L'\\' || dir.back() == L'/')) dir.pop_back();
    if (dir.size() < 4 || PathIsRelativeW(dir.c_str())) {
        MessageBoxW(g.dlg, T(S_SU_BAD_PATH), APP_NAME, MB_ICONWARNING);
        SetFocus(g.edDir);
        return;
    }
    if (!autoStart && !g.installedVer.empty() && g.installedVer != L"?" &&
        CompareVersions(g.installedVer, APP_VERSION) > 0) {
        std::wstring q = Fmt(T(S_SU_DOWNGRADE_Q), g.installedVer.c_str(), APP_VERSION);
        if (MessageBoxW(g.dlg, q.c_str(), APP_NAME, MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    }
    if (!autoStart) {  // elevate when the target needs it
        std::wstring probe = dir;
        while (!probe.empty() && !DirExists(probe)) probe = PathDir(probe);
        if (!IsProcessElevated() && !DirWritable(probe)) {
            if (RelaunchElevated(dir)) {
                EndDialog(g.dlg, IDCANCEL);
                return;
            }
            MessageBoxW(g.dlg, T(S_ERR_PERM), APP_NAME, MB_ICONWARNING);
            return;
        }
    }
    if (!QuitRunningApp(g.dlg, !autoStart)) return;
    g.dir = dir;
    g.installing = true;
    g.progress = 0;
    SetBusy(true);
    SetWindowTextW(g.status, T(S_SU_INSTALLING));
    InvalidateProgress();
    std::thread(InstallWorker, dir, Button_GetCheck(g.chkDesktop) == BST_CHECKED, g_lang).detach();
}

INT_PTR CALLBACK SetupProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_INITDIALOG: {
        g.dlg = dlg;
        bool autoStart = lp != 0;
        int CW = DW - 48;
        Ctl(dlg, L"STATIC", T(S_SU_LANGUAGE), SS_RIGHT | SS_NOPREFIX, DW - 24 - 180 - 70, 62, 64, 20, -1, g_fontSmall);
        g.cbLang = Ctl(dlg, WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, DW - 24 - 180, 58, 180, 320, IDC_LANG);
        {
            std::vector<std::wstring> names;
            for (int i = 0; i < LANG_COUNT; ++i) names.push_back(kLangs[i].native);
            ComboSet(g.cbLang, names, g_lang);
        }
        g.lblLoc = Ctl(dlg, L"STATIC", T(S_SU_LOCATION), SS_LEFT | SS_NOPREFIX, 24, 120, 300, 20, -1, g_fontBold);
        std::wstring startDir = g.presetDir.empty() ? DefaultInstallDir() : g.presetDir;
        g.edDir = Ctl(dlg, L"EDIT", startDir.c_str(), ES_AUTOHSCROLL | WS_TABSTOP, 24, 144, CW - 104, 28, IDC_DIR,
                      nullptr, WS_EX_CLIENTEDGE);
        g.btnBrowse = MakeButton(dlg, IDC_BROWSE, T(S_SET_BROWSE), 0, BTN_SECONDARY, false, DW - 24 - 96, 143, 96, 30);
        g.installedVer = InstalledVersion();
        std::wstring need;
        if (!g.installedVer.empty()) {
            need = Fmt(T(S_SU_UPGRADE_NOTE), g.installedVer.c_str(), APP_VERSION);
        } else {
            need = T(S_SU_NEEDS_NET);
            if (!IsProcessElevated()) need += T(S_SU_ADMIN_NOTE);
        }
        g.lblSpace = Ctl(dlg, L"STATIC", need.c_str(), SS_LEFT | SS_NOPREFIX, 24, 180, CW, 36, -1, g_fontSmall);
        SetPropW(g.lblSpace, L"note", (HANDLE)1);
        g.chkDesktop = Ctl(dlg, L"BUTTON", T(S_SU_DESKTOP), BS_AUTOCHECKBOX | WS_TABSTOP, 24, 212, CW, 24, IDC_DESKTOP);
        Button_SetCheck(g.chkDesktop, g.desktop ? BST_CHECKED : BST_UNCHECKED);
        g.chkLaunch = Ctl(dlg, L"BUTTON", T(S_SU_LAUNCH), BS_AUTOCHECKBOX | WS_TABSTOP, 24, 240, CW, 24, IDC_LAUNCH);
        Button_SetCheck(g.chkLaunch, g.launch ? BST_CHECKED : BST_UNCHECKED);
        g.status = Ctl(dlg, L"STATIC", L"", SS_LEFT | SS_NOPREFIX, 24, 282, CW, 20, -1);
        g.btnOk = MakeButton(dlg, IDOK, T(g.installedVer.empty() ? S_SU_INSTALL : S_SU_UPGRADE), 0, BTN_PRIMARY, true,
                             DW - 24 - 100 - 10 - 130, FOOT + 14, 130, 36);
        g.btnCancel = MakeButton(dlg, IDCANCEL, T(S_BTN_CANCEL), 0, BTN_SECONDARY, true, DW - 24 - 100, FOOT + 14, 100, 36);
        DlgPlace(dlg, DW, DH);
        g.icon = (HICON)LoadImageW(g_inst, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, S(52), S(52), LR_DEFAULTCOLOR);
        SetFocus(g.btnOk);
        if (autoStart) PostMessageW(dlg, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), 0);
        return FALSE;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(dlg, &ps);
        RECT rc;
        GetClientRect(dlg, &rc);
        if (g.icon) DrawIconEx(hdc, S(24), S(24), g.icon, S(52), S(52), 0, nullptr, DI_NORMAL);
        RECT t{S(92), S(24), rc.right - S(24), S(54)};
        DrawTextIn(hdc, APP_NAME, t, clr.text, g_fontHeading, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        RECT s{S(92), S(56), S(DW - 24 - 250), S(80)};
        DrawTextIn(hdc, Fmt(T(S_SU_SUBTITLE), APP_VERSION), s, clr.sub, g_fontSmall,
                   DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        RECT hl{0, S(HEADER), rc.right, S(HEADER) + 1};
        FillSolid(hdc, hl, clr.line);
        RECT clear{0, S(304), rc.right, S(322)};
        FillSolid(hdc, clear, clr.bg);
        if (g.progress >= 0) {
            RECT pa{S(24), S(308), rc.right - S(24), S(316)};
            FillRound(hdc, pa, S(4), clr.track);
            RECT f = pa;
            f.right = f.left + (pa.right - pa.left) * std::min(1000, g.progress) / 1000;
            if (f.right - f.left > S(8)) FillRound(hdc, f, S(4), g.finished ? clr.success : clr.accent);
        }
        RECT foot{0, S(FOOT), rc.right, rc.bottom};
        FillSolid(hdc, foot, clr.panel);
        RECT fl{0, S(FOOT), rc.right, S(FOOT) + 1};
        FillSolid(hdc, fl, clr.line);
        EndPaint(dlg, &ps);
        return TRUE;
    }
    case WM_CTLCOLORDLG:
        return (INT_PTR)BrushOf(clr.bg);
    case WM_CTLCOLORSTATIC:
        return DlgColor((HDC)wp, GetPropW((HWND)lp, L"note") ? clr.sub : clr.text, clr.bg);
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:  // drop-down part of the language combo
        return DlgColor((HDC)wp);
    case WM_DRAWITEM:
        return DrawFancyButton((const DRAWITEMSTRUCT*)lp);
    case WM_SETUP_PROGRESS:
        g.progress = (int)wp;
        InvalidateProgress();
        return TRUE;
    case WM_SETUP_STATUS: {
        std::unique_ptr<std::wstring> text((std::wstring*)lp);
        if (text) SetWindowTextW(g.status, text->c_str());
        return TRUE;
    }
    case WM_SETUP_DONE:
        g.installing = false;
        if (wp == 1) {
            g.finished = true;
            g.progress = 1000;
            SetWindowTextW(g.status, T(S_SU_DONE));
            SetWindowTextW(g.btnOk, T(S_SU_FINISH));
            EnableWindow(g.btnOk, TRUE);
            ShowWindow(g.btnCancel, SW_HIDE);
            SetFocus(g.btnOk);
        } else {
            std::unique_ptr<std::wstring> err((std::wstring*)lp);
            g.progress = -1;
            SetWindowTextW(g.status, L"");
            SetBusy(false);
            MessageBoxW(dlg, Fmt(T(S_SU_FAILED), err ? err->c_str() : L"").c_str(), APP_NAME, MB_ICONERROR);
        }
        InvalidateProgress();
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_LANG:
            if (HIWORD(wp) == CBN_SELCHANGE && !g.installing) {
                int sel = ComboGet(g.cbLang);
                if (sel >= 0 && sel != g_lang) {
                    g_lang = sel;
                    g.desktop = Button_GetCheck(g.chkDesktop) == BST_CHECKED;
                    g.launch = Button_GetCheck(g.chkLaunch) == BST_CHECKED;
                    EndDialog(dlg, RESULT_RESTART);
                }
            }
            return TRUE;
        case IDC_BROWSE: {
            std::wstring p = Trim(GetText(g.edDir));
            std::wstring start = DirExists(p) ? p : PathDir(p);
            if (PickFolder(dlg, start, T(S_SU_PICK_FOLDER))) {
                if (_wcsicmp(PathName(start).c_str(), APP_FOLDER) != 0) start = PathJoin(start, APP_FOLDER);
                SetWindowTextW(g.edDir, start.c_str());
            }
            return TRUE;
        }
        case IDOK:
            if (g.finished) {
                if (Button_GetCheck(g.chkLaunch) == BST_CHECKED) LaunchApp(g.dir);
                EndDialog(dlg, IDOK);
            } else if (!g.installing) {
                StartInstall(false);
            }
            return TRUE;
        case IDCANCEL:
            if (!g.installing) EndDialog(dlg, IDCANCEL);
            return TRUE;
        }
        return FALSE;
    case WM_DESTROY:
        if (g.icon) {
            DestroyIcon(g.icon);
            g.icon = nullptr;
        }
        return FALSE;
    }
    return FALSE;
}

// ---------------------------------------------------------------- uninstall

void SelfDeleteLater(const std::wstring& path) {
    wchar_t sys[MAX_PATH];
    GetSystemDirectoryW(sys, MAX_PATH);
    std::wstring cmd = L"\"" + std::wstring(sys) + L"\\cmd.exe\" /c ping 127.0.0.1 -n 3 > nul & del /f /q \"" + path + L"\"";
    STARTUPINFOW si{sizeof si};
    PROCESS_INFORMATION pi{};
    if (CreateProcessW(nullptr, &cmd[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, sys, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

void DeleteTree(const std::wstring& dir) {
    if (!DirExists(dir)) return;
    std::wstring from = dir;
    from.push_back(L'\0');
    from.push_back(L'\0');
    SHFILEOPSTRUCTW op{};
    op.wFunc = FO_DELETE;
    op.pFrom = from.c_str();
    op.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
    SHFileOperationW(&op);
}

int DoUninstall(const std::wstring& dir, bool purge) {
    std::wstring orig = PathJoin(dir, L"Uninstall.exe");
    for (int i = 0; i < 60 && FileExists(orig) && !DeleteFileW(orig.c_str()); ++i) Sleep(200);
    QuitRunningApp(nullptr, false);

    std::vector<std::wstring> files;
    std::string log;
    if (ReadFileBytes(PathJoin(dir, L"install.log"), log)) {
        for (auto& line : Split(W(log), L'\n')) {
            std::wstring f = Trim(line);
            if (!f.empty() && f.find(L"..") == std::wstring::npos && f.find(L':') == std::wstring::npos) files.push_back(f);
        }
    } else {
        files = {APP_EXE, L"Uninstall.exe", L"install.log", L"default.ini"};
    }
    std::set<std::wstring> dirs;
    for (auto& f : files) {
        DeleteFileW(PathJoin(dir, f).c_str());
        for (std::wstring p = PathDir(f); !p.empty(); p = PathDir(p)) dirs.insert(p);
    }
    DeleteTree(PathJoin(dir, L"tools"));
    DeleteTree(PathJoin(dir, L"licenses"));
    std::vector<std::wstring> sorted(dirs.begin(), dirs.end());
    std::sort(sorted.begin(), sorted.end(), [](const std::wstring& a, const std::wstring& b) { return a.size() > b.size(); });
    for (auto& d : sorted) RemoveDirectoryW(PathJoin(dir, d).c_str());
    RemoveDirectoryW(dir.c_str());

    for (REFKNOWNFOLDERID id : {FOLDERID_Programs, FOLDERID_CommonPrograms, FOLDERID_Desktop, FOLDERID_PublicDesktop}) {
        std::wstring f = KnownFolder(id);
        if (!f.empty()) DeleteFileW(PathJoin(f, SHORTCUT_NAME).c_str());
    }
    RegDeleteTreeW(HKEY_CURRENT_USER, UNINSTALL_SUBKEY);
    RegDeleteTreeW(HKEY_LOCAL_MACHINE, UNINSTALL_SUBKEY);
    if (purge) {
        DeleteTree(AppDataDir());
        DeleteTree(LocalDataDir());
    }
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    MessageBoxW(nullptr, T(S_SU_UNINSTALL_DONE), APP_NAME, MB_ICONINFORMATION);
    return 0;
}

int AskUninstall(const std::wstring& dir) {
    TASKDIALOGCONFIG tc{};
    tc.cbSize = sizeof tc;
    tc.hInstance = g_inst;
    tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
    tc.pszWindowTitle = T(S_SU_UNINSTALL_TITLE);
    tc.pszMainInstruction = T(S_SU_UNINSTALL_Q);
    tc.pszContent = T(S_SU_UNINSTALL_C);
    tc.pszVerificationText = T(S_SU_UNINSTALL_PURGE);
    TASKDIALOG_BUTTON btns[] = {{IDOK, T(S_SU_UNINSTALL_BTN)}, {IDCANCEL, T(S_BTN_CANCEL)}};
    tc.pButtons = btns;
    tc.cButtons = 2;
    tc.nDefaultButton = IDOK;
    int btn = 0;
    BOOL purge = FALSE;
    if (FAILED(TaskDialogIndirect(&tc, &btn, nullptr, &purge)) || btn != IDOK) return 1;

    wchar_t tmpDir[MAX_PATH];
    GetTempPathW(MAX_PATH, tmpDir);
    std::wstring tmp = PathJoin(tmpDir, Fmt(L"YouTubeToPC-uninstall-%lu.exe", GetTickCount()));
    if (!CopyFileW(g.self.c_str(), tmp.c_str(), FALSE)) return DoUninstall(dir, purge != FALSE);
    std::wstring cmd = QuoteArg(tmp) + L" /uninstall-run " + QuoteArg(dir) + L" /lang " + kLangs[g_lang].code +
                       (purge ? L" /purge" : L"");
    STARTUPINFOW si{sizeof si};
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(tmp.c_str(), &cmd[0], nullptr, nullptr, FALSE, 0, nullptr, tmpDir, &si, &pi)) {
        DeleteFileW(tmp.c_str());
        return DoUninstall(dir, purge != FALSE);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}

const wchar_t* ArgValue(int argc, LPWSTR* argv, const wchar_t* name) {
    for (int i = 1; i + 1 < argc; ++i)
        if (_wcsicmp(argv[i], name) == 0) return argv[i + 1];
    return nullptr;
}

bool HasArg(int argc, LPWSTR* argv, const wchar_t* name) {
    for (int i = 1; i < argc; ++i)
        if (_wcsicmp(argv[i], name) == 0) return true;
    return false;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    g_inst = hInst;
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    INITCOMMONCONTROLSEX icc{sizeof icc, ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES};
    InitCommonControlsEx(&icc);
    Gdiplus::GdiplusStartupInput gsi;
    ULONG_PTR token = 0;
    Gdiplus::GdiplusStartup(&token, &gsi, nullptr);

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    // The installer asks for a language; Korean is the default, as requested.
    g_lang = LANG_KO;
    if (const wchar_t* l = ArgValue(argc, argv, L"/lang")) {
        int idx = LangFromCode(l);
        if (idx >= 0) g_lang = idx;
    }
    UiInit();
    ApplyTheme(THEME_SYSTEM);

    wchar_t buf[MAX_PATH * 2];
    GetModuleFileNameW(nullptr, buf, MAX_PATH * 2);
    g.self = buf;
    int rc = 0;

    if (argc >= 3 && _wcsicmp(argv[1], L"/uninstall-run") == 0) {
        rc = DoUninstall(argv[2], HasArg(argc, argv, L"/purge"));
        SelfDeleteLater(g.self);
    } else if (ReadFooter(g.self, g.footer) && !HasArg(argc, argv, L"/uninstall")) {
        const wchar_t* dir = ArgValue(argc, argv, L"/dir");
        bool elevatedRun = dir != nullptr;
        if (elevatedRun) {
            g.presetDir = dir;
            g.desktop = HasArg(argc, argv, L"/desktop");
            g.launch = HasArg(argc, argv, L"/launch");
        }
        for (;;) {
            INT_PTR r = RunDialog(nullptr, APP_NAME, SetupProc, elevatedRun ? 1 : 0);
            if (r != RESULT_RESTART) break;
            UiReloadFonts();  // the new language may need another font
        }
    } else if (FileExists(PathJoin(PathDir(g.self), APP_EXE)) || FileExists(PathJoin(PathDir(g.self), L"install.log"))) {
        rc = AskUninstall(PathDir(g.self));
    } else {
        MessageBoxW(nullptr, T(S_SU_NO_PAYLOAD), APP_NAME, MB_ICONERROR);
        rc = 1;
    }
    if (argv) LocalFree(argv);
    Gdiplus::GdiplusShutdown(token);
    CoUninitialize();
    return rc;
}
