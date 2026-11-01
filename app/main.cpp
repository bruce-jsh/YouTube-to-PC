#include "model.h"
#include "json.h"
#include <thread>
#include <atomic>

namespace {

enum {
    IDC_PASTE = 100,
    IDC_KIND,
    IDC_QUALITY,
    IDC_FORMAT,
    IDC_SUB,
    IDC_AUDIO,
    IDC_DIR,
    IDC_OPENDIR,
    IDC_SETTINGS,
    IDC_FILTER,
    IDC_SORT,
    IDC_LIST,

    IDM_PASTE = 200,
    IDM_OPEN_VIDEO_DIR,
    IDM_OPEN_AUDIO_DIR,
    IDM_EXIT,
    IDM_SELECT_ALL,
    IDM_DELETE,        // drops the list entry only
    IDM_DELETE_FILE,   // drops the entry and sends the file to the Recycle Bin
    IDM_CLEAR_DONE,
    IDM_DELETE_ALL,
    IDM_DELETE_ALL_FILE,
    IDM_TAB0 = 210,
    IDM_SORT0 = 220,
    IDM_SETTINGS = 230,
    // IDM_UPDATE_YTDLP = 231 (model.h)
    IDM_RETRY_ALL = 232,
    IDM_STOP_ALL,
    IDM_ABOUT,
    IDM_APP_UPDATE,
    IDM_PLAY = 300,
    IDM_SHOW_FOLDER,
    IDM_COPY_LINK,
    IDM_OPEN_LINK,
    IDM_DL_PLAYLIST,
    IDM_DL_CHANNEL,
    IDM_TO_MP3,
    IDM_REDOWNLOAD,
    IDM_RETRY,
    IDM_CANCEL,
};

enum { MODE_DEFAULT, MODE_QUICK, MODE_DIALOG };
enum { TIMER_SAVE = 1, TIMER_HINT = 2, TIMER_AUTOUPDATE = 3, TIMER_MENUHOVER = 4, TIMER_MENUWATCH = 5 };
// Long enough that sliding the pointer across the bar on the way somewhere else
// does not fling a menu open.
constexpr UINT kMenuHoverDelay = 250;
constexpr ULONG_PTR COPYDATA_LINKS = 0x59545043;  // 'YTPC'

const int kTabName[5] = {S_TAB_ALL, S_TAB_VIDEO, S_TAB_AUDIO, S_TAB_PLAYLIST, S_TAB_CHANNEL};
const int kSortName[5] = {S_SORT_NEW, S_SORT_OLD, S_SORT_TITLE, S_SORT_SIZE, S_SORT_LENGTH};
const int kMenuName[5] = {S_MENU_FILE, S_MENU_EDIT, S_MENU_VIEW, S_MENU_TOOLS, S_MENU_HELP};

// CharUpper rather than towupper: the C locale leaves Cyrillic alone, which
// would break Alt+Ф for Russian and Mongolian.
wchar_t UpChar(wchar_t c) { return (wchar_t)(UINT_PTR)CharUpperW((LPWSTR)(UINT_PTR)c); }

// The Alt letter is whatever follows the & in the translated title.
wchar_t MenuKey(int i) {
    const wchar_t* s = T(kMenuName[i]);
    for (const wchar_t* p = s; *p; ++p)
        if (*p == L'&' && p[1] && p[1] != L'&') return UpChar(p[1]);
    return 0;
}

HWND g_hwnd, g_btnPaste, g_cbKind, g_cbQuality, g_cbFormat, g_cbSub, g_cbAudio, g_cbDir, g_btnOpenDir, g_btnSettings;
HWND g_filter, g_cbSort, g_list, g_tip;
HICON g_iconBig, g_iconSmall;

std::vector<std::unique_ptr<Item>> g_items;
std::map<int, Item*> g_byId;
int g_nextId = 1;
std::vector<int> g_view;
std::map<int, int> g_viewRow;
int g_tabCounts[5];

int g_hover = -1, g_hoverBtn = 0;
bool g_listTracking = false;
bool g_filterFocus = false;
std::wstring g_hint;
COLORREF g_hintColor = 0;

int g_comboH = 24, g_menuH = 30, g_hdrH = 140, g_tabsY = 140, g_tabsH = 48, g_listY = 190, g_statusH = 28;
RECT g_rcHint, g_rcToggle, g_rcFilter, g_rcStatus, g_rcTabsArea, g_rcTab[5], g_rcMenuBar, g_rcMenu[5];
int g_menuHover = -1, g_menuOpen = -1;
bool g_menuTracking = false;
// While a menu is open the pointer decides what happens next: sliding onto
// another title switches, leaving the bar closes. Keyboard-opened menus stay
// put until the mouse actually moves, so a stray nudge does not shut them.
bool g_menuByMouse = false;
POINT g_menuAnchor{};
std::vector<std::pair<RECT, int>> g_labels;
std::wstring g_statusText;

Tools g_tools;
std::wstring g_ytdlpVer;
int g_versionProc = 0, g_updateProc = 0;
bool g_updateSilent = false;
// Set while the app checks for or downloads a new version of itself; shown on
// the right of the status bar in place of the yt-dlp version.
std::wstring g_appUpdateMsg;
struct PendingLink {
    std::wstring url;
    int mode, forceKind;
};
std::vector<PendingLink> g_pendingLinks;
UINT g_msgQuit = 0;

struct Analysis {
    int itemId;
    LinkInfo link;
    int mode;
    int forceKind;
};
std::map<int, Analysis> g_analyze;
std::map<int, int> g_dlProc;
std::map<int, int> g_convProc;

struct PendingDlg {
    int itemId;
    std::unique_ptr<VideoMeta> meta;
    int forceKind;
};
std::deque<PendingDlg> g_dialogs;
bool g_dialogOpen = false;

int g_batchDone = 0, g_batchFailed = 0;
ITaskbarList3* g_taskbar = nullptr;
UINT g_msgTaskbarCreated = 0;
bool g_trayAdded = false;

void RebuildView();
void UpdateStatus();
void StartQueued();
void AddLink(const std::wstring& raw, int mode, int forceKind);
void Layout();
void RevealItem(Item* it);

// ====================================================================== items

Item* FindItem(int id) {
    auto it = g_byId.find(id);
    return it == g_byId.end() ? nullptr : it->second;
}

Item* NewItem() {
    auto up = std::make_unique<Item>();
    up->id = g_nextId++;
    up->added = NowMs();
    Item* it = up.get();
    g_byId[it->id] = it;
    g_items.push_back(std::move(up));
    return it;
}

bool IsActive(const Item* it) {
    return it->st == St::Analyzing || it->st == St::Choosing || it->st == St::Queued || it->st == St::Downloading ||
           it->st == St::Processing;
}

void RemoveItem(int id) {
    Item* it = FindItem(id);
    if (!it) return;
    if (it->proc) ProcCancel(it->proc);
    if (it->thumb) DeleteObject(it->thumb);
    g_byId.erase(id);
    g_items.erase(std::remove_if(g_items.begin(), g_items.end(), [&](const std::unique_ptr<Item>& p) { return p->id == id; }),
                  g_items.end());
}

void ScheduleSave() { SetTimer(g_hwnd, TIMER_SAVE, 1500, nullptr); }

void InvalidateRow(int row) {
    if (row < 0) return;
    RECT r;
    if (SendMessageW(g_list, LB_GETITEMRECT, row, (LPARAM)&r) != LB_ERR) InvalidateRect(g_list, &r, FALSE);
}

void InvalidateItem(int id) {
    auto it = g_viewRow.find(id);
    if (it != g_viewRow.end()) InvalidateRow(it->second);
}

std::vector<int> SelectedIds() {
    std::vector<int> ids;
    int n = (int)SendMessageW(g_list, LB_GETSELCOUNT, 0, 0);
    if (n <= 0) return ids;
    std::vector<int> rows(n);
    SendMessageW(g_list, LB_GETSELITEMS, n, (LPARAM)rows.data());
    for (int r : rows)
        if (r >= 0 && r < (int)g_view.size()) ids.push_back(g_view[r]);
    return ids;
}

void SelectOnlyRow(int row) {
    SendMessageW(g_list, LB_SETSEL, FALSE, -1);
    if (row >= 0) {
        SendMessageW(g_list, LB_SETSEL, TRUE, row);
        SendMessageW(g_list, LB_SETCARETINDEX, row, FALSE);
    }
}

std::wstring Ellipsize(const std::wstring& s, size_t n) { return s.size() <= n ? s : s.substr(0, n) + L"…"; }

// ====================================================================== notifications

void Notify(const std::wstring& title, const std::wstring& text) {
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof nid;
    nid.hWnd = g_hwnd;
    nid.uID = 1;
    if (!g_trayAdded) {
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        nid.uCallbackMessage = WM_APP_TRAY;
        nid.hIcon = g_iconSmall;
        wcsncpy(nid.szTip, APP_NAME, 63);
        if (!Shell_NotifyIconW(NIM_ADD, &nid)) return;
        nid.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIconW(NIM_SETVERSION, &nid);
        g_trayAdded = true;
    }
    nid.uFlags = NIF_INFO;
    wcsncpy(nid.szInfoTitle, title.c_str(), 63);
    wcsncpy(nid.szInfo, text.c_str(), 255);
    nid.dwInfoFlags = NIIF_INFO | (g_set.sound ? 0 : NIIF_NOSOUND);
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

bool InBackground() { return GetForegroundWindow() != g_hwnd || IsIconic(g_hwnd); }

void CheckBatchFinished() {
    for (auto& up : g_items)
        if (IsActive(up.get())) return;
    if (g_batchDone + g_batchFailed == 0) return;
    if (g_set.notify == 1 && InBackground()) {
        std::wstring text = g_batchFailed ? Fmt(T(S_MSG_DONE_SOME), g_batchDone, g_batchFailed)
                                          : Fmt(T(S_MSG_DONE_ALL), g_batchDone);
        Notify(T(S_MSG_DONE_TITLE), text);
    }
    if (g_set.sound) MessageBeep(MB_ICONASTERISK);
    g_batchDone = g_batchFailed = 0;
}

void UpdateTaskbar() {
    if (!g_taskbar) return;
    int n = 0;
    double sum = 0;
    for (auto& up : g_items) {
        Item* it = up.get();
        if (it->st == St::Downloading) { sum += it->pct; ++n; }
        else if (it->st == St::Processing) { sum += 100; ++n; }
        else if (it->st == St::Queued) ++n;
    }
    if (n == 0) {
        g_taskbar->SetProgressState(g_hwnd, TBPF_NOPROGRESS);
    } else {
        g_taskbar->SetProgressState(g_hwnd, TBPF_NORMAL);
        g_taskbar->SetProgressValue(g_hwnd, (ULONGLONG)sum, (ULONGLONG)(100 * n));
    }
}

// ====================================================================== option bar

void FillDirCombo() {
    std::wstring dir = g_set.kind == KIND_VIDEO ? g_set.dirVideo : g_set.dirAudio;
    ComboSet(g_cbDir, {dir, T(S_DIR_VIDEOS), T(S_DIR_MUSIC), T(S_DIR_DOWNLOADS), T(S_DIR_DOCS), T(S_DIR_DESKTOP), T(S_DIR_BROWSE)}, 0);
    SendMessageW(g_cbDir, CB_SETDROPPEDWIDTH, S(400), 0);
}

void FillBar() {
    ComboSet(g_cbKind, {T(S_KIND_VIDEO), T(S_KIND_AUDIO)}, g_set.kind);
    if (g_set.kind == KIND_VIDEO) {
        ComboSet(g_cbQuality, Labels(kVideoQuality), g_set.vq);
        ComboSet(g_cbFormat, Labels(kVideoFormat), g_set.vfmt);
        EnableWindow(g_cbQuality, TRUE);
    } else {
        ComboSet(g_cbFormat, Labels(kAudioFormat), g_set.afmt);
        if (std::wstring(kAudioFormat[g_set.afmt].value) == L"mp3") {
            ComboSet(g_cbQuality, Labels(kAudioQuality), g_set.aq);
            EnableWindow(g_cbQuality, TRUE);
        } else {
            ComboSet(g_cbQuality, {Fmt(T(S_FMT_ORIGINAL), Upper(kAudioFormat[g_set.afmt].value).c_str())}, 0);
            EnableWindow(g_cbQuality, FALSE);
        }
    }
    ComboSet(g_cbSub, SubtitleLabels(), SubtitleIndexOf(g_set.subLang));
    ComboSet(g_cbAudio, AudioLangLabels(), AudioLangIndexOf(g_set.audioLang));
    SendMessageW(g_cbSub, CB_SETDROPPEDWIDTH, S(200), 0);
    SendMessageW(g_cbAudio, CB_SETDROPPEDWIDTH, S(200), 0);
    FillDirCombo();
}

void ApplyDirChoice(int sel) {
    if (sel <= 0) return;
    std::wstring& dir = g_set.kind == KIND_VIDEO ? g_set.dirVideo : g_set.dirAudio;
    switch (sel) {
    case 1: dir = KnownFolder(FOLDERID_Videos); break;
    case 2: dir = KnownFolder(FOLDERID_Music); break;
    case 3: dir = KnownFolder(FOLDERID_Downloads); break;
    case 4: dir = KnownFolder(FOLDERID_Documents); break;
    case 5: dir = KnownFolder(FOLDERID_Desktop); break;
    case 6: {
        std::wstring p = dir;
        if (PickFolder(g_hwnd, p, T(S_PICK_FOLDER))) dir = p;
        break;
    }
    }
    FillDirCombo();
    SaveSettings();
}

// ====================================================================== layout & painting

void UpdateToolRect() {
    TTTOOLINFOW ti{};
    ti.cbSize = sizeof ti;
    ti.hwnd = g_hwnd;
    ti.uId = 1;
    ti.rect = g_rcToggle;
    SendMessageW(g_tip, TTM_NEWTOOLRECTW, 0, (LPARAM)&ti);
}

void Layout() {
    RECT rc;
    GetClientRect(g_hwnd, &rc);
    int cx = rc.right, cy = rc.bottom;
    if (cx <= 0 || cy <= 0) return;
    HDC hdc = GetDC(g_hwnd);
    int pad = S(20);

    g_menuH = FontHeight(g_font) + S(14);
    g_rcMenuBar = {0, 0, cx, g_menuH};
    int mx = S(10);
    for (int i = 0; i < 5; ++i) {
        int w = TextWidth(hdc, g_font, StripMn(T(kMenuName[i]))) + S(22);
        g_rcMenu[i] = {mx, 0, mx + w, g_menuH};
        mx += w;
    }

    int y1 = g_menuH + S(14), h1 = S(44), btnW = std::max(S(170), TextWidth(hdc, g_fontBold, T(S_BTN_PASTE)) + S(64));
    MoveWindow(g_btnPaste, pad, y1, btnW, h1, FALSE);
    g_rcHint = {pad + btnW + S(18), y1, cx - pad, y1 + h1};

    g_labels.clear();
    int x = pad;
    auto place = [&](int sid, HWND cb, int w, int y) {
        int lw = TextWidth(hdc, g_font, T(sid));
        g_labels.push_back({RECT{x, y, x + lw + 2, y + g_comboH}, sid});
        x += lw + S(8);
        MoveWindow(cb, x, y, w, S(320), FALSE);
        x += w + S(16);
    };
    int y2 = y1 + h1 + S(14);
    place(S_LBL_KIND, g_cbKind, S(86), y2);
    place(S_LBL_QUALITY, g_cbQuality, S(136), y2);
    place(S_LBL_FORMAT, g_cbFormat, S(104), y2);
    place(S_LBL_SUBS, g_cbSub, S(136), y2);
    place(S_LBL_AUDIO, g_cbAudio, S(136), y2);

    int y3 = y2 + g_comboH + S(10);
    int setW = g_comboH + S(8);
    int setX = cx - pad - setW;
    MoveWindow(g_btnSettings, setX, y3, setW, g_comboH, FALSE);
    int toggleW = S(38) + S(8) + TextWidth(hdc, g_fontBold, T(S_TOGGLE_QUICK)) + S(4);
    g_rcToggle = {setX - S(22) - toggleW, y3, setX - S(22), y3 + g_comboH};
    x = pad;
    int lw = TextWidth(hdc, g_font, T(S_LBL_SAVETO));
    g_labels.push_back({RECT{x, y3, x + lw + 2, y3 + g_comboH}, S_LBL_SAVETO});
    x += lw + S(8);
    int openW = g_comboH + S(8);
    int dirW = std::max<int>(S(160), g_rcToggle.left - S(28) - x - openW - S(6));
    MoveWindow(g_cbDir, x, y3, dirW, S(320), FALSE);
    MoveWindow(g_btnOpenDir, x + dirW + S(6), y3, openW, g_comboH, FALSE);
    g_hdrH = y3 + g_comboH + S(16);

    g_tabsY = g_hdrH;
    g_tabsH = S(48);
    g_rcTabsArea = {0, g_tabsY, cx, g_tabsY + g_tabsH};
    int sortW = S(150), filterW = S(220), fh = S(30);
    MoveWindow(g_cbSort, cx - pad - sortW, g_tabsY + (g_tabsH - g_comboH) / 2, sortW, S(300), FALSE);
    int fy = g_tabsY + (g_tabsH - fh) / 2;
    g_rcFilter = {cx - pad - sortW - S(10) - filterW, fy, cx - pad - sortW - S(10), fy + fh};
    int feh = FontHeight(g_font) + S(2);
    MoveWindow(g_filter, g_rcFilter.left + S(32), fy + (fh - feh) / 2, filterW - S(32) - S(8), feh, FALSE);

    g_statusH = S(28);
    g_listY = g_tabsY + g_tabsH + 1;
    g_rcStatus = {0, cy - g_statusH, cx, cy};
    MoveWindow(g_list, 0, g_listY, cx, std::max(0, cy - g_statusH - g_listY), FALSE);
    ReleaseDC(g_hwnd, hdc);
    UpdateToolRect();
    RedrawWindow(g_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME);
}

void PaintMain(HDC hdc) {
    RECT rc;
    GetClientRect(g_hwnd, &rc);
    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HGDIOBJ ob = SelectObject(mem, bmp);

    RECT hdr{0, 0, rc.right, g_hdrH};
    FillSolid(mem, hdr, clr.panel);
    RECT hl{0, g_hdrH - 1, rc.right, g_hdrH};
    FillSolid(mem, hl, clr.line);

    // menu strip (drawn by us so it follows the theme)
    for (int i = 0; i < 5; ++i) {
        bool on = i == g_menuOpen || (g_menuOpen < 0 && i == g_menuHover);
        if (on) {
            RECT r = g_rcMenu[i];
            InflateRect(&r, 0, -S(3));
            FillRound(mem, r, S(4), clr.hover);
        }
        DrawTextMn(mem, T(kMenuName[i]), g_rcMenu[i], on ? clr.text : clr.sub, g_font, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    if (!g_hint.empty()) {
        DrawTextIn(mem, g_hint, g_rcHint, g_hintColor, g_fontBold, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    } else {
        RECT ir = g_rcHint;
        ir.right = ir.left + S(24);
        DrawTextIn(mem, std::wstring(1, ico::Link), ir, clr.faint, g_fontIcon, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        RECT tr = g_rcHint;
        tr.left += S(28);
        DrawTextIn(mem, T(S_HINT_DEFAULT), tr, clr.sub, g_font, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    for (auto& l : g_labels) DrawTextIn(mem, T(l.second), l.first, clr.sub, g_font, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    {   // "instant download" toggle
        RECT r = g_rcToggle;
        int cy = (r.top + r.bottom) / 2;
        RECT tr{r.left, cy - S(10), r.left + S(38), cy + S(10)};
        FillRound(mem, tr, S(10), g_set.quick ? clr.accent : clr.borderStrong);
        int kx = g_set.quick ? tr.right - S(18) : tr.left + S(2);
        RECT knob{kx, cy - S(8), kx + S(16), cy + S(8)};
        FillEllipse(mem, knob, RGB(255, 255, 255));
        RECT lr{tr.right + S(8), r.top, r.right + S(4), r.bottom};
        DrawTextIn(mem, T(S_TOGGLE_QUICK), lr, g_set.quick ? clr.text : clr.sub, g_fontBold, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    FillSolid(mem, g_rcTabsArea, clr.bg);
    int x = S(20);
    for (int i = 0; i < 5; ++i) {
        bool sel = i == g_set.tab;
        std::wstring name = T(kTabName[i]), cnt = std::to_wstring(g_tabCounts[i]);
        int nw = TextWidth(mem, g_fontBold, name), cw = TextWidth(mem, g_fontSmall, cnt);
        RECT r{x, g_tabsY, x + S(10) + nw + S(5) + cw + S(10), g_tabsY + g_tabsH};
        g_rcTab[i] = r;
        RECT nr{r.left + S(10), r.top, r.left + S(10) + nw + 2, r.bottom - S(2)};
        DrawTextIn(mem, name, nr, sel ? clr.text : clr.sub, sel ? g_fontBold : g_font, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        RECT cr{nr.right + S(4), r.top, r.right, r.bottom - S(2)};
        DrawTextIn(mem, cnt, cr, sel ? clr.accent : clr.faint, g_fontSmall, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        if (sel) {
            RECT u{r.left + S(8), r.bottom - S(3), r.right - S(8), r.bottom};
            FillRound(mem, u, S(1), clr.accent);
        }
        x = r.right + S(4);
    }
    FillRound(mem, g_rcFilter, S(6), clr.bg, g_filterFocus ? clr.accent : clr.border, g_filterFocus ? 1.6f : 1.0f);
    RECT sr{g_rcFilter.left + S(10), g_rcFilter.top, g_rcFilter.left + S(32), g_rcFilter.bottom};
    DrawTextIn(mem, std::wstring(1, ico::Search), sr, clr.faint, g_fontIconSmall, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    RECT tl{0, g_tabsY + g_tabsH, rc.right, g_listY};
    FillSolid(mem, tl, clr.line);

    FillSolid(mem, g_rcStatus, clr.panel);
    RECT stl{0, g_rcStatus.top, rc.right, g_rcStatus.top + 1};
    FillSolid(mem, stl, clr.line);
    RECT st1{S(20), g_rcStatus.top, rc.right / 2, g_rcStatus.bottom};
    DrawTextIn(mem, g_statusText, st1, clr.sub, g_fontSmall, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    std::wstring right;
    COLORREF rcColor = clr.faint;
    if (!g_appUpdateMsg.empty()) {
        right = g_appUpdateMsg;
    } else if (g_tools.ytdlp.empty()) {
        right = T(S_STATUS_NO_YTDLP);
        rcColor = clr.danger;
    } else if (g_updateProc) {
        right = T(S_STATUS_UPDATING);
    } else {
        right = L"yt-dlp " + (g_ytdlpVer.empty() ? std::wstring(L"…") : g_ytdlpVer);
        if (g_tools.ffmpegExe.empty()) {
            right += T(S_STATUS_NO_FFMPEG);
            rcColor = clr.danger;
        }
    }
    RECT st2{rc.right / 2, g_rcStatus.top, rc.right - S(20), g_rcStatus.bottom};
    DrawTextIn(mem, right, st2, rcColor, g_fontSmall, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, ob);
    DeleteObject(bmp);
    DeleteDC(mem);
}

// ---------------------------------------------------------------------- list drawing

void RowButtonRects(int w, int h, RECT& folder, RECT& more) {
    int sz = S(32), r = w - S(20);
    more = {r - sz, (h - sz) / 2, r, (h + sz) / 2};
    folder = {more.left - S(4) - sz, more.top, more.left - S(4), more.bottom};
}

std::wstring MetaLine(const Item* it, COLORREF& color) {
    color = clr.sub;
    std::vector<std::wstring> parts;
    auto add = [&](const std::wstring& s) {
        if (!s.empty()) parts.push_back(s);
    };
    switch (it->st) {
    case St::Analyzing: add(T(S_ST_ANALYZING)); break;
    case St::Choosing:
        add(T(S_ST_CHOOSING));
        color = clr.accent;
        break;
    case St::Downloading:
        if (it->opt.kind == KIND_VIDEO) add(T(it->part == 1 ? S_ST_DL_AUDIO : S_ST_DL_VIDEO));
        else add(T(S_ST_DL));
        if (it->total > 0) add(FormatSize(it->done) + L" / " + FormatSize(it->total));
        if (it->speed > 0) add(FormatSize((long long)it->speed) + L"/s");
        if (it->eta >= 0) add(Fmt(T(S_ST_REMAIN), FormatEta(it->eta).c_str()));
        break;
    case St::Processing: add(T(it->opt.kind == KIND_VIDEO ? S_ST_MERGING : S_ST_CONVERTING)); break;
    case St::Done:
        add(FormatSize(it->size));
        add(it->info);
        add(it->channel);
        break;
    case St::Error:
        add(it->err.empty() ? T(S_ST_FAILED_MSG) : it->err);
        color = clr.danger;
        break;
    case St::Queued:
    case St::Canceled:
        add(OptSummary(it->opt));
        add(it->channel);
        break;
    }
    if (it->groupType == GROUP_PLAYLIST && !it->groupTitle.empty() && (it->st == St::Done || it->st == St::Queued))
        add(Fmt(T(S_PLAYLIST_OF), it->groupTitle.c_str()));
    std::wstring s;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) s += L"  ·  ";
        s += parts[i];
    }
    return s;
}

void DrawStatus(HDC mem, const Item* it, RECT st) {
    int h = st.bottom - st.top, cy = h / 2;
    auto iconText = [&](wchar_t icon, const std::wstring& text, COLORREF c) {
        int tw = TextWidth(mem, g_fontBold, text);
        RECT tr{st.right - tw - 2, 0, st.right, h};
        DrawTextIn(mem, text, tr, c, g_fontBold, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        if (icon) {
            std::wstring ic(1, icon);
            int iw = TextWidth(mem, g_fontIconSmall, ic);
            RECT ir{tr.left - S(6) - iw, 0, tr.left - S(6), h};
            DrawTextIn(mem, ic, ir, c, g_fontIconSmall, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    };
    auto bar = [&](double pct, COLORREF fill) {
        RECT b{st.left + S(10), cy + S(6), st.right, cy + S(12)};
        FillRound(mem, b, S(3), clr.track);
        int fw = (int)((b.right - b.left) * std::max(0.0, std::min(100.0, pct)) / 100.0);
        if (fw > S(5)) {
            RECT f = b;
            f.right = f.left + fw;
            FillRound(mem, f, S(3), fill);
        }
    };
    RECT top{st.left, cy - S(22), st.right, cy + S(2)};
    switch (it->st) {
    case St::Downloading:
        DrawTextIn(mem, Fmt(L"%.0f%%", it->pct), top, clr.text, g_fontBold, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
        bar(it->pct, clr.accent);
        break;
    case St::Processing:
        DrawTextIn(mem, T(S_ST_PROCESSING), top, clr.accent, g_fontBold, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
        bar(100, clr.accentMuted);
        break;
    case St::Queued: iconText(ico::Clock, T(S_ST_QUEUED), clr.faint); break;
    case St::Analyzing: iconText(0, T(S_ST_CHECKING), clr.faint); break;
    case St::Choosing: iconText(0, T(S_ST_PICK), clr.accent); break;
    case St::Done: iconText(ico::Check, T(S_ST_DONE), clr.success); break;
    case St::Error: iconText(ico::Error, T(S_ST_ERROR), clr.danger); break;
    case St::Canceled: iconText(ico::Pause, T(S_ST_STOPPED), clr.faint); break;
    }
}

void DrawListItem(const DRAWITEMSTRUCT* d) {
    if ((int)d->itemID < 0) return;
    Item* it = FindItem((int)d->itemData);
    RECT rc = d->rcItem;
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    HDC mem = CreateCompatibleDC(d->hDC);
    HBITMAP bmp = CreateCompatibleBitmap(d->hDC, w, h);
    HGDIOBJ ob = SelectObject(mem, bmp);
    bool sel = d->itemState & ODS_SELECTED, hov = (int)d->itemID == g_hover;
    COLORREF bg = sel ? clr.accentSoft : hov ? clr.hover : clr.bg;
    RECT r{0, 0, w, h};
    FillSolid(mem, r, bg);
    if (it) {
        RECT sep{S(20), h - 1, w - S(20), h};
        FillSolid(mem, sep, clr.line);
        int tw = S(112), th = S(63);
        RECT tr{S(20), (h - th) / 2, S(20) + tw, (h - th) / 2 + th};
        if (it->thumb) {
            HDC td = CreateCompatibleDC(mem);
            HGDIOBJ o = SelectObject(td, it->thumb);
            BitBlt(mem, tr.left, tr.top, tw, th, td, 0, 0, SRCCOPY);
            SelectObject(td, o);
            DeleteDC(td);
        } else {
            FillRound(mem, tr, S(4), clr.track);
            DrawTextIn(mem, std::wstring(1, it->opt.kind == KIND_AUDIO ? ico::Music : ico::Video), tr, clr.faint, g_fontIcon,
                       DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            if (!it->thumbReq && !it->vid.empty()) {
                it->thumbReq = true;
                ThumbRequest(it->vid, g_hwnd, (WPARAM)it->id, tw, th);
            }
        }
        if (it->duration > 0) {
            std::wstring ds = FormatDuration(it->duration);
            int bw = TextWidth(mem, g_fontSmall, ds) + S(10);
            RECT b{tr.right - bw - S(4), tr.bottom - S(18) - S(4), tr.right - S(4), tr.bottom - S(4)};
            FillRound(mem, b, S(3), RGB(24, 24, 28));
            DrawTextIn(mem, ds, b, RGB(255, 255, 255), g_fontSmall, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        int right = w - S(20);
        if (hov && it->st != St::Analyzing) {
            RECT rf, rm;
            RowButtonRects(w, h, rf, rm);
            auto ghost = [&](const RECT& br, wchar_t glyph, bool hot) {
                if (hot) FillRound(mem, br, S(6), clr.track);
                DrawTextIn(mem, std::wstring(1, glyph), br, hot ? clr.text : clr.sub, g_fontIconSmall,
                           DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            };
            ghost(rm, ico::More, g_hoverBtn == 2);
            right = rm.left - S(8);
            if (it->st == St::Done) {
                ghost(rf, ico::Folder, g_hoverBtn == 1);
                right = rf.left - S(8);
            }
        }
        RECT st{right - S(150), 0, right, h};
        DrawStatus(mem, it, st);

        int tx = tr.right + S(16), txr = st.left - S(16);
        RECT t1{tx, S(13), txr, S(13) + S(26)};
        bool badge = it->isNew && it->st == St::Done;
        RECT tc = t1;
        int badgeW = 0;
        if (badge) {
            badgeW = TextWidth(mem, g_fontSmall, T(S_BADGE_NEW)) + S(14);
            int titleW = TextWidth(mem, g_fontTitle, it->title);
            tc.right = std::min<LONG>(t1.right - badgeW - S(10), t1.left + titleW + 2);
        }
        DrawTextIn(mem, it->title, tc, clr.text, g_fontTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        if (badge) {
            RECT b{tc.right + S(8), t1.top + S(5), tc.right + S(8) + badgeW, t1.bottom - S(5)};
            FillRound(mem, b, S(8), clr.accent);
            DrawTextIn(mem, T(S_BADGE_NEW), b, RGB(255, 255, 255), g_fontSmall, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        COLORREF mc;
        std::wstring meta = MetaLine(it, mc);
        RECT ic{tx, S(42), tx + S(18), S(42) + S(22)};
        DrawTextIn(mem, std::wstring(1, it->opt.kind == KIND_AUDIO ? ico::Music : ico::Video), ic, clr.faint, g_fontIconSmall,
                   DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        RECT t2{tx + S(22), S(42), txr, S(42) + S(22)};
        DrawTextIn(mem, meta, t2, mc, g_font, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
    BitBlt(d->hDC, rc.left, rc.top, w, h, mem, 0, 0, SRCCOPY);
    SelectObject(mem, ob);
    DeleteObject(bmp);
    DeleteDC(mem);
}

void PaintEmpty(HWND h, HDC hdc) {
    RECT rc;
    GetClientRect(h, &rc);
    FillSolid(hdc, rc, clr.bg);
    std::wstring head, sub1, sub2;
    COLORREF headColor = clr.text;
    if (g_tools.ytdlp.empty()) {
        head = T(S_EMPTY_NOTOOL_HEAD);
        sub1 = T(S_EMPTY_NOTOOL_SUB);
        headColor = clr.danger;
    } else if (!g_items.empty()) {
        head = GetText(g_filter).empty() ? T(S_EMPTY_NONE_HEAD) : T(S_EMPTY_SEARCH_HEAD);
        sub1 = T(S_EMPTY_NONE_SUB);
    } else {
        head = T(S_EMPTY_HEAD);
        sub1 = T(S_EMPTY_SUB1);
        sub2 = T(S_EMPTY_SUB2);
    }
    int cy = rc.bottom / 2 - S(30);
    RECT ir{0, cy - S(70), rc.right, cy - S(6)};
    DrawTextIn(hdc, std::wstring(1, ico::Download), ir, clr.border, g_fontIconLarge, DT_CENTER | DT_BOTTOM | DT_SINGLELINE);
    RECT hr{0, cy + S(6), rc.right, cy + S(36)};
    DrawTextIn(hdc, head, hr, headColor, g_fontHeading, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    RECT s1{0, cy + S(42), rc.right, cy + S(64)};
    DrawTextIn(hdc, sub1, s1, clr.sub, g_font, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    RECT s2{0, cy + S(66), rc.right, cy + S(88)};
    DrawTextIn(hdc, sub2, s2, clr.sub, g_font, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

int HitRow(HWND h, POINT pt) {
    LRESULT r = SendMessageW(h, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
    if (HIWORD(r)) return -1;
    int row = LOWORD(r);
    return row < (int)g_view.size() ? row : -1;
}

int HoverButtonAt(POINT pt) {
    if (g_hover < 0 || g_hover >= (int)g_view.size()) return 0;
    Item* it = FindItem(g_view[g_hover]);
    if (!it || it->st == St::Analyzing) return 0;
    RECT ir;
    if (SendMessageW(g_list, LB_GETITEMRECT, g_hover, (LPARAM)&ir) == LB_ERR) return 0;
    RECT rf, rm;
    RowButtonRects(ir.right - ir.left, ir.bottom - ir.top, rf, rm);
    OffsetRect(&rf, ir.left, ir.top);
    OffsetRect(&rm, ir.left, ir.top);
    if (PtInRect(&rm, pt)) return 2;
    if (it->st == St::Done && PtInRect(&rf, pt)) return 1;
    return 0;
}

void ShowContextMenu(POINT screen);

LRESULT CALLBACK ListProc(HWND h, UINT m, WPARAM w, LPARAM l, UINT_PTR, DWORD_PTR) {
    switch (m) {
    case WM_PAINT:
        if (SendMessageW(h, LB_GETCOUNT, 0, 0) == 0) {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(h, &ps);
            PaintEmpty(h, hdc);
            EndPaint(h, &ps);
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        if (SendMessageW(h, LB_GETCOUNT, 0, 0) == 0) return 1;
        break;
    case WM_SIZE:
        InvalidateRect(h, nullptr, TRUE);
        break;
    case WM_MOUSEMOVE: {
        POINT pt{GET_X_LPARAM(l), GET_Y_LPARAM(l)};
        int row = HitRow(h, pt);
        if (row != g_hover) {
            int old = g_hover;
            g_hover = row;
            g_hoverBtn = 0;
            InvalidateRow(old);
            InvalidateRow(row);
        }
        int hb = HoverButtonAt(pt);
        if (hb != g_hoverBtn) {
            g_hoverBtn = hb;
            InvalidateRow(g_hover);
        }
        if (!g_listTracking) {
            TRACKMOUSEEVENT t{sizeof t, TME_LEAVE, h, 0};
            TrackMouseEvent(&t);
            g_listTracking = true;
        }
        break;
    }
    case WM_MOUSELEAVE:
        g_listTracking = false;
        if (g_hover >= 0) {
            int old = g_hover;
            g_hover = -1;
            g_hoverBtn = 0;
            InvalidateRow(old);
        }
        break;
    case WM_MOUSEWHEEL: {
        LRESULT r = DefSubclassProc(h, m, w, l);
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(h, &pt);
        int row = HitRow(h, pt);
        if (row != g_hover) {
            int old = g_hover;
            g_hover = row;
            InvalidateRow(old);
            InvalidateRow(row);
        }
        return r;
    }
    case WM_LBUTTONDOWN: {
        POINT pt{GET_X_LPARAM(l), GET_Y_LPARAM(l)};
        int btn = HoverButtonAt(pt);
        if (btn) {
            int row = g_hover;
            Item* it = FindItem(g_view[row]);
            if (btn == 1 && it) {
                RevealItem(it);
            } else {
                std::vector<int> sel = SelectedIds();
                if (std::find(sel.begin(), sel.end(), g_view[row]) == sel.end()) SelectOnlyRow(row);
                RECT ir;
                SendMessageW(h, LB_GETITEMRECT, row, (LPARAM)&ir);
                RECT rf, rm;
                RowButtonRects(ir.right - ir.left, ir.bottom - ir.top, rf, rm);
                POINT p{ir.left + rm.left, ir.top + rm.bottom};
                ClientToScreen(h, &p);
                ShowContextMenu(p);
            }
            return 0;
        }
        break;
    }
    case WM_NCDESTROY:
        RemoveWindowSubclass(h, ListProc, 1);
        break;
    }
    return DefSubclassProc(h, m, w, l);
}

// ====================================================================== view

void RebuildView() {
    std::wstring q = Trim(GetText(g_filter));
    std::vector<Item*> v;
    int counts[5] = {0, 0, 0, 0, 0};
    for (auto& up : g_items) {
        Item* it = up.get();
        bool match[5] = {true, it->opt.kind == KIND_VIDEO, it->opt.kind == KIND_AUDIO, it->groupType == GROUP_PLAYLIST,
                         it->groupType == GROUP_CHANNEL};
        for (int k = 0; k < 5; ++k) counts[k] += match[k];
        if (!match[g_set.tab]) continue;
        if (!q.empty() && !ContainsI(it->title, q) && !ContainsI(it->channel, q) && !ContainsI(it->groupTitle, q)) continue;
        v.push_back(it);
    }
    switch (g_set.sort) {
    case 0:
        std::stable_sort(v.begin(), v.end(), [](Item* a, Item* b) { return a->added != b->added ? a->added > b->added : a->id < b->id; });
        break;
    case 1:
        std::stable_sort(v.begin(), v.end(), [](Item* a, Item* b) { return a->added != b->added ? a->added < b->added : a->id < b->id; });
        break;
    case 2:
        std::stable_sort(v.begin(), v.end(), [](Item* a, Item* b) {
            return CompareStringEx(LOCALE_NAME_USER_DEFAULT, NORM_IGNORECASE | SORT_DIGITSASNUMBERS, a->title.c_str(), -1,
                                   b->title.c_str(), -1, nullptr, nullptr, 0) == CSTR_LESS_THAN;
        });
        break;
    case 3:
        std::stable_sort(v.begin(), v.end(), [](Item* a, Item* b) { return a->size > b->size; });
        break;
    case 4:
        std::stable_sort(v.begin(), v.end(), [](Item* a, Item* b) { return a->duration > b->duration; });
        break;
    }
    std::vector<int> selIds = SelectedIds();
    std::set<int> sel(selIds.begin(), selIds.end());
    int top = (int)SendMessageW(g_list, LB_GETTOPINDEX, 0, 0);
    int topId = (top >= 0 && top < (int)g_view.size()) ? g_view[top] : 0;

    SendMessageW(g_list, WM_SETREDRAW, FALSE, 0);
    SendMessageW(g_list, LB_RESETCONTENT, 0, 0);
    SendMessageW(g_list, LB_INITSTORAGE, v.size(), 0);
    g_view.clear();
    g_viewRow.clear();
    int newTop = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        SendMessageW(g_list, LB_ADDSTRING, 0, (LPARAM)v[i]->id);
        g_view.push_back(v[i]->id);
        g_viewRow[v[i]->id] = (int)i;
        if (v[i]->id == topId) newTop = (int)i;
    }
    for (size_t i = 0; i < v.size(); ++i)
        if (sel.count(v[i]->id)) SendMessageW(g_list, LB_SETSEL, TRUE, (LPARAM)i);
    SendMessageW(g_list, LB_SETTOPINDEX, newTop, 0);
    SendMessageW(g_list, WM_SETREDRAW, TRUE, 0);
    g_hover = -1;
    InvalidateRect(g_list, nullptr, TRUE);
    memcpy(g_tabCounts, counts, sizeof counts);
    InvalidateRect(g_hwnd, &g_rcTabsArea, FALSE);
    UpdateStatus();
}

void SelectAndShow(int id) {
    auto it = g_viewRow.find(id);
    if (it == g_viewRow.end()) return;
    SelectOnlyRow(it->second);
    SendMessageW(g_list, LB_SETTOPINDEX, std::max(0, it->second - 1), 0);
}

void UpdateStatus() {
    int active = 0, queued = 0, done = 0, failed = 0;
    for (auto& up : g_items) {
        switch (up->st) {
        case St::Analyzing:
        case St::Downloading:
        case St::Processing: ++active; break;
        case St::Queued: ++queued; break;
        case St::Done: ++done; break;
        case St::Error: ++failed; break;
        default: break;
        }
    }
    g_statusText = Fmt(T(S_STATUS_COUNTS), active, queued, done);
    if (failed) g_statusText += Fmt(T(S_STATUS_FAILED), failed);
    InvalidateRect(g_hwnd, &g_rcStatus, FALSE);
    UpdateTaskbar();
}

// ====================================================================== downloads

void StartQueued() {
    if (g_tools.ytdlp.empty() || g_updateProc) return;
    int limit = std::max(1, std::min(kMaxConcurrent, g_set.concurrency));
    int running = (int)g_dlProc.size();
    for (auto& up : g_items) {
        if (running >= limit) break;
        Item* it = up.get();
        if (it->st != St::Queued) continue;
        it->gotDone = false;
        it->lastError.clear();
        it->err.clear();
        it->pct = 0;
        it->done = it->total = 0;
        it->speed = 0;
        it->eta = -1;
        int pid = ProcStart(g_tools.ytdlp, DownloadArgs(g_tools, *it), g_hwnd, true);
        if (!pid) {
            it->st = St::Error;
            it->err = T(S_ERR_RUN_YTDLP);
            InvalidateItem(it->id);
            continue;
        }
        it->proc = pid;
        it->st = St::Downloading;
        g_dlProc[pid] = it->id;
        ++running;
        InvalidateItem(it->id);
    }
    UpdateStatus();
}

void OnDownloadLine(Item* it, const std::wstring& s) {
    if (StartsWith(s, L"[P]")) {
        auto f = Split(s.substr(3), L'|');
        if (f.size() < 6) return;
        auto num = [](const std::wstring& x) { return (x.empty() || x == L"NA" || x == L"None") ? -1.0 : _wtof(x.c_str()); };
        double dl = num(f[0]), tot = num(f[1]), est = num(f[2]);
        if (tot <= 0) tot = est;
        it->part = f[5] == L"none" ? 1 : 0;
        it->done = (long long)std::max(0.0, dl);
        it->total = (long long)std::max(0.0, tot);
        it->pct = tot > 0 ? std::min(100.0, std::max(0.0, dl) * 100.0 / tot) : 0;
        it->speed = num(f[3]);
        it->eta = num(f[4]);
        St ns = (tot > 0 && dl >= tot) ? St::Processing : St::Downloading;
        bool changed = ns != it->st;
        it->st = ns;
        DWORD now = GetTickCount();
        if (changed || now - it->lastPaint > 150) {
            it->lastPaint = now;
            InvalidateItem(it->id);
            UpdateTaskbar();
        }
    } else if (StartsWith(s, L"[DONE]")) {
        auto f = Split(s.substr(6), L'|');
        if (f.size() < 5) return;
        std::wstring path = f[4];
        for (size_t i = 5; i < f.size(); ++i) path += L"|" + f[i];
        it->file = path;
        it->size = FileSize(path);
        it->info = BuildInfo(it->opt, f[0], f[1], f[2], f[3]);
        it->gotDone = true;
    } else if (s.find(L"ERROR") != std::wstring::npos) {
        it->lastError += s + L"\n";
    }
}

void OnDownloadExit(int itemId, const ProcExit* pe) {
    Item* it = FindItem(itemId);
    if (it) {
        it->proc = 0;
        if (it->gotDone) {
            it->st = St::Done;
            it->isNew = true;
            it->pct = 100;
            it->err.clear();
            ++g_batchDone;
            if (g_set.notify == 2 && InBackground()) Notify(T(S_MSG_DONE_TITLE), it->title);
        } else if (pe->canceled) {
            it->st = St::Canceled;
        } else {
            it->st = St::Error;
            it->err = FriendlyError(it->lastError.empty() ? W(pe->output) : it->lastError);
            ++g_batchFailed;
            if (g_set.notifyErrors && InBackground()) Notify(T(S_MSG_FAIL_TITLE), it->title + L"\n" + it->err);
        }
        InvalidateItem(it->id);
        ScheduleSave();
    }
    StartQueued();
    CheckBatchFinished();
}

void AddEntries(const VideoMeta& m, const DlOptions& base) {
    long long now = NowMs();
    DlOptions o = base;
    if (g_set.groupFolders && !m.title.empty()) o.dir = PathJoin(o.dir, SanitizeFileName(m.title));
    int first = 0;
    for (auto& e : m.entries) {
        if (!e.checked) continue;
        Item* it = NewItem();
        it->added = now;
        it->url = e.url;
        it->vid = e.vid;
        it->title = e.title;
        it->channel = e.channel;
        it->channelUrl = e.channelUrl;
        it->duration = e.duration;
        it->groupType = m.groupType;
        it->groupTitle = m.title;
        it->groupUrl = m.url;
        it->opt = o;
        it->st = St::Queued;
        if (!first) first = it->id;
    }
    RebuildView();
    if (first) SelectAndShow(first);
    StartQueued();
    ScheduleSave();
}

bool ConfirmGroup(const VideoMeta& m) {
    TASKDIALOGCONFIG tc{};
    tc.cbSize = sizeof tc;
    tc.hwndParent = g_hwnd;
    tc.hInstance = g_inst;
    tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
    tc.pszWindowTitle = APP_NAME;
    std::wstring main = Fmt(T(m.groupType == GROUP_CHANNEL ? S_MSG_CONFIRM_CHANNEL : S_MSG_CONFIRM_PLAYLIST), (int)m.entries.size());
    tc.pszMainInstruction = main.c_str();
    tc.pszContent = m.title.c_str();
    TASKDIALOG_BUTTON btns[] = {{IDOK, T(S_MSG_DL_ALL)}, {IDCANCEL, T(S_BTN_CANCEL)}};
    tc.pButtons = btns;
    tc.cButtons = 2;
    tc.nDefaultButton = IDOK;
    int btn = 0;
    return SUCCEEDED(TaskDialogIndirect(&tc, &btn, nullptr, nullptr)) && btn == IDOK;
}

void PumpDialogs() {
    if (g_dialogOpen || g_dialogs.empty()) return;
    PendingDlg p = std::move(g_dialogs.front());
    g_dialogs.pop_front();
    if (FindItem(p.itemId)) {
        DlOptions opt = OptionsFromBar(p.forceKind);
        g_dialogOpen = true;
        bool ok = ShowOptionsDialog(g_hwnd, *p.meta, opt);
        g_dialogOpen = false;
        Item* ph = FindItem(p.itemId);
        if (ph) {
            if (ok && p.meta->isGroup) {
                RemoveItem(ph->id);
                AddEntries(*p.meta, opt);
            } else if (ok) {
                ph->opt = opt;
                ph->st = St::Queued;
                RebuildView();
                StartQueued();
            } else {
                RemoveItem(ph->id);
                RebuildView();
            }
            ScheduleSave();
        }
    }
    if (!g_dialogs.empty()) PostMessageW(g_hwnd, WM_APP_NEXT_DIALOG, 0, 0);
}

void OnAnalyzeExit(const Analysis& an, const ProcExit* pe) {
    Item* ph = FindItem(an.itemId);
    if (!ph) return;
    ph->proc = 0;
    if (pe->canceled) {
        RemoveItem(ph->id);
        RebuildView();
        return;
    }
    auto meta = std::make_unique<VideoMeta>();
    if (!ParseMeta(pe->output, an.link, *meta)) {
        ph->st = St::Error;
        ph->err = FriendlyError(W(pe->output));
        ++g_batchFailed;
        InvalidateItem(ph->id);
        UpdateStatus();
        ScheduleSave();
        CheckBatchFinished();
        return;
    }
    bool quick = an.mode == MODE_QUICK || (an.mode == MODE_DEFAULT && g_set.quick);
    if (meta->isGroup) {
        ph->title = meta->title;
        ph->channel = meta->channel;
        ph->groupType = meta->groupType;
        ph->groupTitle = meta->title;
        if (!meta->vid.empty() && meta->vid != ph->vid) {
            ph->vid = meta->vid;
            ph->thumbReq = false;
        }
        if (meta->entries.empty()) {
            ph->st = St::Error;
            ph->err = T(S_MSG_NO_VIDEOS);
            RebuildView();
            ScheduleSave();
            return;
        }
        if (quick) {
            if (meta->entries.size() > 20 && !ConfirmGroup(*meta)) {
                RemoveItem(ph->id);
                RebuildView();
                return;
            }
            RemoveItem(ph->id);
            AddEntries(*meta, OptionsFromBar(an.forceKind));
            return;
        }
    } else {
        if (!meta->title.empty()) ph->title = meta->title;
        ph->channel = meta->channel;
        ph->channelUrl = meta->channelUrl;
        ph->duration = meta->duration;
        if (!meta->url.empty()) ph->url = meta->url;
        if (!meta->vid.empty() && meta->vid != ph->vid) {
            ph->vid = meta->vid;
            ph->thumbReq = false;
        }
        if (quick) {
            ph->opt = OptionsFromBar(an.forceKind);
            ph->st = St::Queued;
            RebuildView();
            StartQueued();
            return;
        }
    }
    ph->st = St::Choosing;
    RebuildView();
    g_dialogs.push_back(PendingDlg{ph->id, std::move(meta), an.forceKind});
    PostMessageW(g_hwnd, WM_APP_NEXT_DIALOG, 0, 0);
}

void AddLink(const std::wstring& raw, int mode, int forceKind) {
    if (g_tools.ytdlp.empty()) {
        MessageBoxW(g_hwnd, T(S_MSG_NO_YTDLP), APP_NAME, MB_ICONWARNING);
        return;
    }
    if (g_updateProc) {
        g_pendingLinks.push_back(PendingLink{raw, mode, forceKind});
        g_statusText = T(S_STATUS_WAIT_UPDATE);
        InvalidateRect(g_hwnd, &g_rcStatus, FALSE);
        return;
    }
    LinkInfo li = ClassifyLink(raw);
    if (li.url.empty()) return;
    Item* it = NewItem();
    it->url = li.url;
    it->vid = li.vid;
    it->title = li.url;
    it->st = St::Analyzing;
    it->opt = OptionsFromBar(forceKind);
    if (li.type == LINK_PLAYLIST) it->groupType = GROUP_PLAYLIST;
    if (li.type == LINK_CHANNEL) it->groupType = GROUP_CHANNEL;
    if (li.type == LINK_VIDEO && !li.list.empty()) it->playlistUrl = L"https://www.youtube.com/playlist?list=" + li.list;
    int pid = ProcStart(g_tools.ytdlp, AnalyzeArgs(g_tools, li.url, li.type == LINK_VIDEO), g_hwnd, false);
    if (!pid) {
        it->st = St::Error;
        it->err = T(S_ERR_RUN_YTDLP);
    } else {
        it->proc = pid;
        g_analyze[pid] = Analysis{it->id, li, mode, forceKind};
    }
    RebuildView();
    SelectAndShow(it->id);
}

void ShowHint(const std::wstring& text, COLORREF color) {
    g_hint = text;
    g_hintColor = color;
    InvalidateRect(g_hwnd, &g_rcHint, FALSE);
    SetTimer(g_hwnd, TIMER_HINT, text.empty() ? 1 : 5000, nullptr);
}

// The main action: take whatever link is on the clipboard and start downloading.
void PasteLinks() {
    std::wstring clip = GetClipboardText(g_hwnd);
    auto links = ExtractLinks(clip);
    if (links.empty()) {
        ShowHint(T(Trim(clip).empty() ? S_HINT_CLIP_EMPTY : S_HINT_CLIP_NOLINK), clr.danger);
        MessageBeep(MB_ICONWARNING);
        return;
    }
    for (auto& l : links) AddLink(l, MODE_DEFAULT, -1);
    if (links.size() > 1) ShowHint(Fmt(T(S_HINT_GOT_N), (int)links.size()), clr.accent);
    else ShowHint(L"", clr.sub);
}

void ExtractMp3(Item* src) {
    if (g_tools.ffmpegExe.empty()) {
        MessageBoxW(g_hwnd, T(S_MSG_NO_FFMPEG), APP_NAME, MB_ICONWARNING);
        return;
    }
    if (!FileExists(src->file)) {
        MessageBoxW(g_hwnd, T(S_MSG_NO_SOURCE), APP_NAME, MB_ICONWARNING);
        return;
    }
    std::wstring base = PathJoin(PathDir(src->file), PathStem(src->file));
    std::wstring out = base + L".mp3";
    for (int i = 2; FileExists(out); ++i) out = base + Fmt(L" (%d).mp3", i);
    Item* it = NewItem();
    it->url = src->url;
    it->vid = src->vid;
    it->title = src->title;
    it->channel = src->channel;
    it->channelUrl = src->channelUrl;
    it->duration = src->duration;
    it->groupType = src->groupType;
    it->groupTitle = src->groupTitle;
    it->groupUrl = src->groupUrl;
    it->opt = src->opt;
    it->opt.kind = KIND_AUDIO;
    it->opt.afmt = L"mp3";
    it->opt.aq = L"0";
    it->file = out;
    it->st = St::Processing;
    std::wstring args = L"-hide_banner -loglevel error -y -i " + QuoteArg(src->file) +
                        L" -vn -map 0:a:0 -c:a libmp3lame -q:a 0 -map_metadata 0 " + QuoteArg(out);
    int pid = ProcStart(g_tools.ffmpegExe, args, g_hwnd, false);
    if (!pid) {
        it->st = St::Error;
        it->err = T(S_ERR_RUN_FFMPEG);
    } else {
        it->proc = pid;
        g_convProc[pid] = it->id;
    }
    RebuildView();
    SelectAndShow(it->id);
}

void OnConvertExit(int itemId, const ProcExit* pe) {
    Item* it = FindItem(itemId);
    if (!it) return;
    it->proc = 0;
    if (pe->code == 0 && FileExists(it->file)) {
        it->st = St::Done;
        it->isNew = true;
        it->size = FileSize(it->file);
        it->info = L"MP3 · VBR";
    } else {
        DeleteFileW(it->file.c_str());
        it->st = pe->canceled ? St::Canceled : St::Error;
        std::wstring out = Trim(W(pe->output));
        it->err = out.empty() ? T(S_ERR_EXTRACT) : std::wstring(T(S_ERR_EXTRACT)) + L" " + Ellipsize(Split(out, L'\n').back(), 120);
    }
    InvalidateItem(it->id);
    UpdateStatus();
    ScheduleSave();
}

void RetryItems(const std::vector<int>& ids) {
    for (int id : ids) {
        Item* it = FindItem(id);
        if (!it || IsActive(it)) continue;
        it->st = St::Queued;
        it->err.clear();
        it->isNew = false;
    }
    RebuildView();
    StartQueued();
}

void CancelItems(const std::vector<int>& ids) {
    for (int id : ids) {
        Item* it = FindItem(id);
        if (!it) continue;
        if (it->st == St::Queued) it->st = St::Canceled;
        else if (it->proc) ProcCancel(it->proc);
        InvalidateItem(id);
    }
    UpdateStatus();
    ScheduleSave();
}

std::vector<int> AllIds() {
    std::vector<int> all;
    for (auto& up : g_items) all.push_back(up->id);
    return all;
}

// True when at least one of these items actually has a file on disk, which is
// what decides whether offering "delete the file too" makes any sense.
bool AnyFileOnDisk(const std::vector<int>& ids) {
    for (int id : ids) {
        Item* it = FindItem(id);
        if (it && it->st == St::Done && FileExists(it->file)) return true;
    }
    return false;
}

// withFiles says which of the two menu entries the user picked: the list alone,
// or the list and the downloaded files. Nothing is hidden behind a checkbox.
void DeleteItems(const std::vector<int>& ids, bool all, bool withFiles) {
    if (ids.empty()) return;
    if (withFiles && !AnyFileOnDisk(ids)) withFiles = false;
    TASKDIALOGCONFIG tc{};
    tc.cbSize = sizeof tc;
    tc.hwndParent = g_hwnd;
    tc.hInstance = g_inst;
    tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
    tc.pszWindowTitle = APP_NAME;
    int n = (int)ids.size();
    std::wstring main;
    if (withFiles)
        main = all ? Fmt(T(S_MSG_DELF_ALL), n) : n == 1 ? std::wstring(T(S_MSG_DELF_ONE)) : Fmt(T(S_MSG_DELF_N), n);
    else
        main = all ? Fmt(T(S_MSG_DEL_ALL), n) : n == 1 ? std::wstring(T(S_MSG_DEL_ONE)) : Fmt(T(S_MSG_DEL_N), n);
    tc.pszMainInstruction = main.c_str();
    tc.pszContent = T(withFiles ? S_MSG_DELF_CONTENT : S_MSG_DEL_CONTENT);
    TASKDIALOG_BUTTON btns[] = {{IDOK, T(S_CM_DELETE)}, {IDCANCEL, T(S_BTN_CANCEL)}};
    tc.pButtons = btns;
    tc.cButtons = 2;
    tc.nDefaultButton = withFiles ? IDCANCEL : IDOK;  // losing files deserves a deliberate click
    int btn = 0;
    if (FAILED(TaskDialogIndirect(&tc, &btn, nullptr, nullptr)) || btn != IDOK) return;
    std::vector<std::wstring> files;
    for (int id : ids) {
        Item* it = FindItem(id);
        if (!it) continue;
        if (withFiles && it->st == St::Done && !it->file.empty()) files.push_back(it->file);
        RemoveItem(id);
    }
    if (!files.empty()) RecycleFiles(files);
    RebuildView();
    ScheduleSave();
    StartQueued();
}

void DeleteAllItems(bool withFiles) { DeleteItems(AllIds(), true, withFiles); }

// Shows a download in Explorer. The file may be gone, moved, or on a folder the
// user has since deleted, so fall back step by step instead of handing Explorer
// a path it cannot resolve, which makes it open Documents instead.
void RevealItem(Item* it) {
    if (!it) return;
    if (FileExists(it->file)) {
        ShowInFolder(it->file);
        return;
    }
    std::wstring dir = PathDir(it->file);
    if (!DirExists(dir)) dir = it->opt.dir;
    if (DirExists(dir)) ShellOpen(dir);
    else MessageBoxW(GetLastActivePopup(g_hwnd), T(S_MSG_FILE_MISSING), APP_NAME, MB_ICONINFORMATION);
}

void OpenSelected() {
    auto ids = SelectedIds();
    if (ids.empty()) return;
    Item* it = FindItem(ids[0]);
    if (!it) return;
    if (it->st == St::Done) {
        if (FileExists(it->file)) {
            if (!ShellOpen(it->file))
                MessageBoxW(g_hwnd, Fmt(T(S_MSG_CANNOT_OPEN), it->file.c_str()).c_str(), APP_NAME, MB_ICONINFORMATION);
            if (it->isNew) {
                it->isNew = false;
                InvalidateItem(it->id);
                ScheduleSave();
            }
        } else {
            MessageBoxW(g_hwnd, T(S_MSG_FILE_MISSING), APP_NAME, MB_ICONINFORMATION);
        }
    } else if (it->st == St::Error) {
        MessageBoxW(g_hwnd, (it->title + L"\n\n" + it->err).c_str(), T(S_MSG_FAIL_TITLE), MB_ICONWARNING);
    }
}

// ====================================================================== tools

void RunVersionCheck() {
    g_ytdlpVer.clear();
    if (!g_tools.ytdlp.empty()) g_versionProc = ProcStart(g_tools.ytdlp, L"--version", g_hwnd, false);
    InvalidateRect(g_hwnd, &g_rcStatus, FALSE);
}

// When the app lives in Program Files the bundled yt-dlp cannot be replaced, so
// updates are kept in the user profile instead.
bool EnsureWritableYtdlp() {
    if (g_tools.ytdlp.empty() || g_tools.ownCopy) return g_tools.ownCopy;
    std::wstring dir = UserToolsDir();
    EnsureDir(dir);
    if (!DirWritable(dir)) return false;
    std::wstring dst = PathJoin(dir, L"yt-dlp.exe");
    if (!FileExists(dst) && !CopyFileW(g_tools.ytdlp.c_str(), dst.c_str(), FALSE)) return false;
    std::wstring qjsDst = PathJoin(dir, L"qjs.exe");
    if (!g_tools.qjs.empty() && !FileExists(qjsDst)) CopyFileW(g_tools.qjs.c_str(), qjsDst.c_str(), FALSE);
    g_tools = FindTools();
    return g_tools.ownCopy;
}

bool RunUpdate(bool silent) {
    if (g_tools.ytdlp.empty()) {
        if (!silent) MessageBoxW(GetLastActivePopup(g_hwnd), T(S_STATUS_NO_YTDLP), APP_NAME, MB_ICONWARNING);
        return false;
    }
    if (g_updateProc) return false;
    if (!g_dlProc.empty()) {
        if (!silent) MessageBoxW(GetLastActivePopup(g_hwnd), T(S_MSG_UPDATE_BUSY), APP_NAME, MB_ICONINFORMATION);
        return false;
    }
    if (!EnsureWritableYtdlp()) {
        if (!silent) MessageBoxW(GetLastActivePopup(g_hwnd), T(S_ERR_PERM), APP_NAME, MB_ICONWARNING);
        return false;
    }
    g_updateProc = ProcStart(g_tools.ytdlp, L"-U", g_hwnd, false);
    if (!g_updateProc) return false;
    g_updateSilent = silent;
    InvalidateRect(g_hwnd, &g_rcStatus, FALSE);
    return true;
}

// Keeps yt-dlp current: YouTube changes often and old versions stop working.
bool MaybeAutoUpdate() {
    if (!g_set.autoUpdate || g_tools.ytdlp.empty()) return false;
    long long now = NowMs();
    if (now - g_set.lastUpdateCheck < 20LL * 3600 * 1000) return false;
    g_set.lastUpdateCheck = now;
    SaveSettings();
    return RunUpdate(true);
}

void OnUpdateExit(const ProcExit* pe) {
    std::wstring out = W(pe->output);
    bool updated = out.find(L"Updated yt-dlp to") != std::wstring::npos;
    bool current = out.find(L"is up to date") != std::wstring::npos;
    RunVersionCheck();
    std::vector<PendingLink> pending;
    pending.swap(g_pendingLinks);
    for (auto& p : pending) AddLink(p.url, p.mode, p.forceKind);
    StartQueued();
    UpdateStatus();
    if (g_updateSilent) return;
    std::wstring msg = updated ? T(S_MSG_UPDATED) : current ? T(S_MSG_UPTODATE) : Fmt(T(S_MSG_UPDATE_FAILED), Ellipsize(Trim(out), 600).c_str());
    MessageBoxW(GetLastActivePopup(g_hwnd), msg.c_str(), APP_NAME, updated || current ? MB_ICONINFORMATION : MB_ICONWARNING);
}

// ====================================================================== appearance

void ApplyAppearance(bool languageChanged) {
    ApplyTheme(g_set.theme);
    if (languageChanged) {
        g_lang = g_set.lang;
        UiReloadFonts();
        SendMessageW(g_btnPaste, WM_SETFONT, (WPARAM)g_fontBold, FALSE);
        for (HWND h : {g_cbKind, g_cbQuality, g_cbFormat, g_cbSub, g_cbAudio, g_cbDir, g_cbSort, g_filter, g_btnOpenDir, g_btnSettings})
            SendMessageW(h, WM_SETFONT, (WPARAM)g_font, FALSE);
        SetWindowTextW(g_btnPaste, T(S_BTN_PASTE));
        SendMessageW(g_filter, EM_SETCUEBANNER, TRUE, (LPARAM)T(S_FILTER_CUE));
        FillBar();
        std::vector<std::wstring> sorts;
        for (int i = 0; i < 5; ++i) sorts.push_back(T(kSortName[i]));
        ComboSet(g_cbSort, sorts, g_set.sort);
        TTTOOLINFOW ti{};
        ti.cbSize = sizeof ti;
        ti.hwnd = g_hwnd;
        struct { UINT_PTR id; int sid; } tips[] = {
            {1, S_TIP_QUICK}, {(UINT_PTR)g_btnPaste, S_TIP_PASTE}, {(UINT_PTR)g_btnOpenDir, S_TIP_OPENDIR},
            {(UINT_PTR)g_btnSettings, S_TIP_SETTINGS},
        };
        for (auto& t : tips) {
            ti.uId = t.id;
            ti.uFlags = t.id == 1 ? TTF_SUBCLASS : (TTF_SUBCLASS | TTF_IDISHWND);
            ti.lpszText = (LPWSTR)T(t.sid);
            SendMessageW(g_tip, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
        }
    }
    ThemeWindow(g_hwnd, true);
    for (HWND h : {g_cbKind, g_cbQuality, g_cbFormat, g_cbSub, g_cbAudio, g_cbDir, g_cbSort}) ThemeControl(h, L"combo");
    ThemeControl(g_list, L"list");
    ThemeControl(g_filter, nullptr);
    Layout();
    RebuildView();
}

// ====================================================================== menus

HMENU BuildPopup(int index) {
    HMENU m = CreatePopupMenu();
    auto add = [&](int sid, int id, const wchar_t* accel = nullptr) {
        std::wstring s = T(sid);
        if (accel) s += std::wstring(L"\t") + accel;
        AppendMenuW(m, MF_STRING, id, s.c_str());
    };
    switch (index) {
    case 0:
        add(S_MI_PASTE, IDM_PASTE, L"Ctrl+V");
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_MI_OPEN_VIDEO_DIR, IDM_OPEN_VIDEO_DIR);
        add(S_MI_OPEN_AUDIO_DIR, IDM_OPEN_AUDIO_DIR);
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_MI_EXIT, IDM_EXIT);
        break;
    case 1: {
        std::vector<int> sel = SelectedIds();
        add(S_MI_SELECT_ALL, IDM_SELECT_ALL, L"Ctrl+A");
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_CM_DEL_LIST, IDM_DELETE, L"Del");
        add(S_CM_DEL_FILE, IDM_DELETE_FILE, L"Shift+Del");
        if (sel.empty()) EnableMenuItem(m, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);
        if (!AnyFileOnDisk(sel)) EnableMenuItem(m, IDM_DELETE_FILE, MF_BYCOMMAND | MF_GRAYED);
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_MI_CLEAR_DONE, IDM_CLEAR_DONE);
        add(S_MI_DELETE_ALL, IDM_DELETE_ALL);
        add(S_MI_DELETE_ALL_FILES, IDM_DELETE_ALL_FILE);
        if (g_items.empty()) {
            EnableMenuItem(m, IDM_CLEAR_DONE, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(m, IDM_DELETE_ALL, MF_BYCOMMAND | MF_GRAYED);
        }
        if (!AnyFileOnDisk(AllIds())) EnableMenuItem(m, IDM_DELETE_ALL_FILE, MF_BYCOMMAND | MF_GRAYED);
        break;
    }
    case 2: {
        for (int i = 0; i < 5; ++i) AppendMenuW(m, MF_STRING, IDM_TAB0 + i, T(kTabName[i]));
        CheckMenuRadioItem(m, IDM_TAB0, IDM_TAB0 + 4, IDM_TAB0 + g_set.tab, MF_BYCOMMAND);
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        HMENU so = CreatePopupMenu();
        for (int i = 0; i < 5; ++i) AppendMenuW(so, MF_STRING, IDM_SORT0 + i, T(kSortName[i]));
        CheckMenuRadioItem(so, IDM_SORT0, IDM_SORT0 + 4, IDM_SORT0 + g_set.sort, MF_BYCOMMAND);
        AppendMenuW(m, MF_POPUP, (UINT_PTR)so, T(S_MI_SORT));
        break;
    }
    case 3:
        add(S_MI_SETTINGS, IDM_SETTINGS, L"Ctrl+,");
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_MI_RETRY_ALL, IDM_RETRY_ALL);
        add(S_MI_STOP_ALL, IDM_STOP_ALL);
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_MI_UPDATE, IDM_UPDATE_YTDLP);
        break;
    default:
        add(S_MI_APP_UPDATE, IDM_APP_UPDATE);
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_MI_ABOUT, IDM_ABOUT);
        break;
    }
    return m;
}

void OpenMenu(int index, bool byMouse) {
    if (index < 0 || index > 4) return;
    g_menuByMouse = byMouse;
    GetCursorPos(&g_menuAnchor);
    KillTimer(g_hwnd, TIMER_MENUHOVER);
    for (;;) {
        g_menuOpen = index;
        InvalidateRect(g_hwnd, &g_rcMenuBar, FALSE);
        UpdateWindow(g_hwnd);
        HMENU m = BuildPopup(index);
        POINT p{g_rcMenu[index].left, g_rcMenu[index].bottom};
        ClientToScreen(g_hwnd, &p);
        // TrackPopupMenu runs its own loop but still dispatches our timer, which
        // is how the pointer can switch or close the menu while it is up.
        SetTimer(g_hwnd, TIMER_MENUWATCH, 60, nullptr);
        int cmd = TrackPopupMenu(m, TPM_RETURNCMD | TPM_LEFTBUTTON, p.x, p.y, 0, g_hwnd, nullptr);
        KillTimer(g_hwnd, TIMER_MENUWATCH);
        DestroyMenu(m);
        g_menuOpen = -1;
        InvalidateRect(g_hwnd, &g_rcMenuBar, FALSE);
        if (cmd) {
            SendMessageW(g_hwnd, WM_COMMAND, cmd, 0);
            return;
        }
        // moving or clicking onto another title switches to it
        POINT cur;
        GetCursorPos(&cur);
        ScreenToClient(g_hwnd, &cur);
        int next = -1;
        for (int i = 0; i < 5; ++i)
            if (i != index && PtInRect(&g_rcMenu[i], cur)) next = i;
        if (next < 0) return;
        index = next;
        g_menuByMouse = true;
    }
}

// Decides, while a menu is open, whether the pointer has moved somewhere that
// should switch menus or close them. Returns true when the menu should end.
bool MenuShouldYield() {
    static int away = 0;
    POINT cur;
    GetCursorPos(&cur);
    if (!g_menuByMouse) {
        // Opened from the keyboard: ignore the mouse until it really moves, so
        // a knock of the desk does not shut the menu.
        if (abs(cur.x - g_menuAnchor.x) < 8 && abs(cur.y - g_menuAnchor.y) < 8) return false;
        g_menuByMouse = true;
    }
    HWND under = WindowFromPoint(cur);
    wchar_t cls[32] = {};
    if (under) GetClassNameW(under, cls, 32);
    if (wcscmp(cls, L"#32768") == 0) {  // on the popup or one of its submenus
        away = 0;
        return false;
    }
    POINT c = cur;
    ScreenToClient(g_hwnd, &c);
    for (int i = 0; i < 5; ++i)
        if (PtInRect(&g_rcMenu[i], c)) {
            away = 0;
            return i != g_menuOpen;  // sliding onto a neighbour switches at once
        }
    // Off the bar and off the popup. Wait a moment before closing: cutting the
    // corner on the way down to a long menu passes through dead space.
    return ++away >= 5;
}

void ShowContextMenu(POINT pt) {
    auto ids = SelectedIds();
    HMENU m = CreatePopupMenu();
    auto add = [&](int sid, int id, UINT flags = MF_STRING, const wchar_t* accel = nullptr) {
        std::wstring s = T(sid);
        if (accel) s += std::wstring(L"\t") + accel;
        AppendMenuW(m, flags, id, s.c_str());
    };
    // The two ways to delete are always side by side, so it is plain which one
    // only tidies the list and which one also throws the file away.
    auto addDeleteAll = [&] {
        add(S_MI_DELETE_ALL, IDM_DELETE_ALL, MF_STRING | (g_items.empty() ? MF_GRAYED : 0));
        add(S_MI_DELETE_ALL_FILES, IDM_DELETE_ALL_FILE, MF_STRING | (AnyFileOnDisk(AllIds()) ? 0 : MF_GRAYED));
    };
    if (ids.empty()) {
        add(S_MI_PASTE, IDM_PASTE, MF_STRING, L"Ctrl+V");
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        addDeleteAll();
    } else if (ids.size() == 1) {
        Item* it = FindItem(ids[0]);
        if (!it) {
            DestroyMenu(m);
            return;
        }
        bool done = it->st == St::Done;
        add(S_CM_PLAY, IDM_PLAY, MF_STRING | (done ? 0 : MF_GRAYED), L"Enter");
        add(S_CM_SHOW_FOLDER, IDM_SHOW_FOLDER, MF_STRING | (done ? 0 : MF_GRAYED));
        if (done) SetMenuDefaultItem(m, IDM_PLAY, FALSE);
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_CM_COPY_LINK, IDM_COPY_LINK);
        add(S_CM_OPEN_LINK, IDM_OPEN_LINK);
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_MI_PASTE, IDM_PASTE, MF_STRING, L"Ctrl+V");
        std::wstring pl = it->groupType == GROUP_PLAYLIST ? it->groupUrl : it->playlistUrl;
        if (!pl.empty() && it->st != St::Analyzing) {
            std::wstring label = (it->groupType == GROUP_PLAYLIST && !it->groupTitle.empty())
                                     ? Fmt(T(S_CM_DL_PLAYLIST_OF), Ellipsize(it->groupTitle, 32).c_str())
                                     : std::wstring(T(S_CM_DL_PLAYLIST));
            AppendMenuW(m, MF_STRING, IDM_DL_PLAYLIST, label.c_str());
        }
        if (!it->channelUrl.empty())
            AppendMenuW(m, MF_STRING, IDM_DL_CHANNEL,
                        Fmt(T(S_CM_DL_CHANNEL), Ellipsize(it->channel.empty() ? it->channelUrl : it->channel, 32).c_str()).c_str());
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        HMENU tools = CreatePopupMenu();
        AppendMenuW(tools, MF_STRING | ((done && it->opt.kind == KIND_VIDEO) ? 0 : MF_GRAYED), IDM_TO_MP3, T(S_CM_TO_MP3));
        AppendMenuW(tools, MF_STRING | (it->st == St::Analyzing ? MF_GRAYED : 0), IDM_REDOWNLOAD, T(S_CM_REDOWNLOAD));
        AppendMenuW(m, MF_POPUP, (UINT_PTR)tools, T(S_CM_MEDIA_TOOLS));
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        if (IsActive(it) && it->st != St::Choosing) add(S_CM_CANCEL_DL, IDM_CANCEL);
        else if (!IsActive(it)) add(done ? S_CM_REDO : S_CM_RETRY, IDM_RETRY);
        add(S_CM_DEL_LIST, IDM_DELETE, MF_STRING, L"Del");
        add(S_CM_DEL_FILE, IDM_DELETE_FILE, MF_STRING | (AnyFileOnDisk(ids) ? 0 : MF_GRAYED), L"Shift+Del");
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        addDeleteAll();
    } else {
        AppendMenuW(m, MF_STRING, IDM_COPY_LINK, Fmt(T(S_CM_COPY_LINK_N), (int)ids.size()).c_str());
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        add(S_CM_REDO, IDM_RETRY);
        add(S_CM_CANCEL_DL, IDM_CANCEL);
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(m, MF_STRING, IDM_DELETE, (Fmt(T(S_CM_DEL_LIST_N), (int)ids.size()) + L"\tDel").c_str());
        AppendMenuW(m, MF_STRING | (AnyFileOnDisk(ids) ? 0 : MF_GRAYED), IDM_DELETE_FILE,
                    (Fmt(T(S_CM_DEL_FILE_N), (int)ids.size()) + L"\tShift+Del").c_str());
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        addDeleteAll();
    }
    int cmd = TrackPopupMenu(m, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, g_hwnd, nullptr);
    DestroyMenu(m);
    if (cmd) SendMessageW(g_hwnd, WM_COMMAND, cmd, 0);
}

void SetTab(int tab) {
    g_set.tab = tab;
    SendMessageW(g_list, LB_SETSEL, FALSE, -1);
    RebuildView();
    InvalidateRect(g_hwnd, &g_rcTabsArea, FALSE);
    SaveSettings();
}

void OpenSettings() {
    bool uiChanged = false;
    if (!ShowSettingsDialog(g_hwnd, g_tools, g_ytdlpVer, uiChanged)) return;
    SaveSettings();
    std::wstring oldYtdlp = g_tools.ytdlp;
    g_tools = FindTools();
    if (g_tools.ytdlp != oldYtdlp) RunVersionCheck();
    ApplyAppearance(uiChanged);
    StartQueued();
}

// ====================================================================== self-update
//
// The installer already knows how to upgrade in place and leaves settings.ini
// and the download history untouched, so updating means fetching the current
// release and handing it the folder this copy runs from.

constexpr const wchar_t* kReleaseApi = L"https://api.github.com/repos/bruce-jsh/YouTube-to-PC/releases/latest";

struct UpdateInfo {
    std::wstring version;  // tag without the leading v
    std::wstring url;      // installer asset
    std::wstring file;     // where it was saved
};

enum { UPD_IDLE, UPD_CHECKING, UPD_DOWNLOADING };
int g_updState = UPD_IDLE;
UpdateInfo g_upd;
std::atomic<bool> g_updCancel{false};

void SetUpdateMsg(const std::wstring& s) {
    g_appUpdateMsg = s;
    InvalidateRect(g_hwnd, &g_rcStatus, FALSE);
}

// Picks the .exe asset out of the release JSON. Anything else means the release
// was published differently and we should not guess.
bool ParseRelease(const std::string& json, UpdateInfo& out) {
    JValue root;
    if (!JParser(json).parse(root) || root.t != JValue::Obj) return false;
    std::wstring tag = W(root["tag_name"].str());
    if (!tag.empty() && (tag[0] == L'v' || tag[0] == L'V')) tag.erase(0, 1);
    if (tag.empty()) return false;
    const JValue& assets = root["assets"];
    for (auto& a : assets.a) {
        std::wstring name = W(a["name"].str());
        if (name.size() > 4 && Lower(name).rfind(L".exe") == name.size() - 4) {
            out.url = W(a["browser_download_url"].str());
            break;
        }
    }
    if (out.url.empty()) return false;
    out.version = tag;
    return true;
}

void StartInstaller() {
    std::wstring args = L"/dir " + QuoteArg(ExeDir()) + L" /lang " + kLangs[g_set.lang].code + L" /launch";
    SHELLEXECUTEINFOW ei{sizeof ei};
    ei.fMask = SEE_MASK_NOCLOSEPROCESS;
    // /dir makes setup install straight away, which skips its own elevation
    // step, so ask for the rights here when the folder is not ours to write.
    ei.lpVerb = (IsProcessElevated() || DirWritable(ExeDir())) ? L"open" : L"runas";
    ei.lpFile = g_upd.file.c_str();
    ei.lpParameters = args.c_str();
    ei.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&ei)) {
        MessageBoxW(GetLastActivePopup(g_hwnd), T(S_UPD_START_FAILED), APP_NAME, MB_ICONWARNING);
        return;
    }
    if (ei.hProcess) CloseHandle(ei.hProcess);
    // Setup asks us to quit and waits, so there is nothing to do but let it.
}

void OnUpdateReady() {
    SetUpdateMsg(L"");
    TASKDIALOGCONFIG tc{};
    tc.cbSize = sizeof tc;
    tc.hwndParent = g_hwnd;
    tc.hInstance = g_inst;
    tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
    tc.pszWindowTitle = APP_NAME;
    tc.pszMainInstruction = T(S_UPD_READY);
    tc.pszContent = T(S_UPD_READY_BODY);
    TASKDIALOG_BUTTON btns[] = {{IDOK, T(S_UPD_INSTALL)}, {IDCANCEL, T(S_UPD_LATER)}};
    tc.pButtons = btns;
    tc.cButtons = 2;
    tc.nDefaultButton = IDOK;
    int btn = 0;
    if (SUCCEEDED(TaskDialogIndirect(&tc, &btn, nullptr, nullptr)) && btn == IDOK) StartInstaller();
}

// Installers from earlier updates are of no further use once a newer one is on
// the way, and they are over a megabyte each.
void RemoveOldInstallers(const std::wstring& dir, const std::wstring& keep) {
    WIN32_FIND_DATAW fd{};
    HANDLE h = FindFirstFileW(PathJoin(dir, L"YouTubeToPC-Setup-*.exe").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (_wcsicmp(fd.cFileName, keep.c_str()) != 0) DeleteFileW(PathJoin(dir, fd.cFileName).c_str());
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

void StartUpdateDownload() {
    g_updState = UPD_DOWNLOADING;
    g_updCancel = false;
    SetUpdateMsg(Fmt(T(S_UPD_DOWNLOADING), 0));
    std::wstring url = g_upd.url;
    std::wstring name = L"YouTubeToPC-Setup-" + g_upd.version + L".exe";
    std::wstring dest = PathJoin(LocalDataDir(), name);
    EnsureDir(LocalDataDir());
    RemoveOldInstallers(LocalDataDir(), name);
    HWND hwnd = g_hwnd;
    std::thread([url, dest, hwnd] {
        int last = -1;
        bool ok = DownloadToFile(url, dest, [&](long long done, long long total) {
            if (g_updCancel) return false;
            int pct = total > 0 ? (int)(done * 100 / total) : 0;
            if (pct != last) {
                last = pct;
                PostMessageW(hwnd, WM_APP_UPDATE_PROGRESS, pct, 0);
            }
            return true;
        });
        PostMessageW(hwnd, WM_APP_UPDATE_DONE, ok ? 1 : 0, (LPARAM) new std::wstring(ok ? dest : L""));
    }).detach();
}

void OnUpdateChecked(bool ok) {
    g_updState = UPD_IDLE;
    SetUpdateMsg(L"");
    HWND owner = GetLastActivePopup(g_hwnd);
    if (!ok) {
        MessageBoxW(owner, T(S_UPD_CHECK_FAILED), APP_NAME, MB_ICONWARNING);
        return;
    }
    if (CompareVersions(g_upd.version, APP_VERSION) <= 0) {
        MessageBoxW(owner, Fmt(T(S_UPD_LATEST), APP_VERSION).c_str(), APP_NAME, MB_ICONINFORMATION);
        return;
    }
    TASKDIALOGCONFIG tc{};
    tc.cbSize = sizeof tc;
    tc.hwndParent = g_hwnd;
    tc.hInstance = g_inst;
    tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW | TDF_USE_HICON_MAIN;
    tc.hMainIcon = g_iconBig;
    tc.pszWindowTitle = APP_NAME;
    std::wstring head = Fmt(T(S_UPD_FOUND), g_upd.version.c_str());
    std::wstring body = Fmt(T(S_UPD_FOUND_BODY), APP_VERSION);
    tc.pszMainInstruction = head.c_str();
    tc.pszContent = body.c_str();
    TASKDIALOG_BUTTON btns[] = {{IDOK, T(S_UPD_NOW)}, {IDCANCEL, T(S_UPD_LATER)}};
    tc.pButtons = btns;
    tc.cButtons = 2;
    tc.nDefaultButton = IDOK;
    int btn = 0;
    if (SUCCEEDED(TaskDialogIndirect(&tc, &btn, nullptr, nullptr)) && btn == IDOK) StartUpdateDownload();
}

void CheckAppUpdate() {
    if (g_updState != UPD_IDLE) {
        MessageBoxW(GetLastActivePopup(g_hwnd), T(S_UPD_BUSY), APP_NAME, MB_ICONINFORMATION);
        return;
    }
    g_updState = UPD_CHECKING;
    SetUpdateMsg(T(S_UPD_CHECKING));
    HWND hwnd = g_hwnd;
    std::thread([hwnd] {
        std::string body;
        auto* info = new UpdateInfo;
        bool ok = HttpGetText(kReleaseApi, body) && ParseRelease(body, *info);
        PostMessageW(hwnd, WM_APP_UPDATE_CHECKED, ok ? 1 : 0, (LPARAM)info);
    }).detach();
}

void ShowAbout() {
    TASKDIALOGCONFIG tc{};
    tc.cbSize = sizeof tc;
    tc.hwndParent = g_hwnd;
    tc.hInstance = g_inst;
    tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW | TDF_USE_HICON_MAIN;
    tc.pszWindowTitle = APP_NAME;
    tc.hMainIcon = g_iconBig;
    std::wstring main = std::wstring(APP_NAME) + L"  " + APP_VERSION;
    tc.pszMainInstruction = main.c_str();
    tc.pszContent = T(S_ABOUT_TEXT);
    tc.dwCommonButtons = TDCBF_OK_BUTTON;
    TaskDialogIndirect(&tc, nullptr, nullptr, nullptr);
}

void OnMenuCommand(int id) {
    auto ids = SelectedIds();
    Item* first = ids.empty() ? nullptr : FindItem(ids[0]);
    switch (id) {
    case IDM_PASTE: PasteLinks(); break;
    case IDM_OPEN_VIDEO_DIR: EnsureDir(g_set.dirVideo); ShellOpen(g_set.dirVideo); break;
    case IDM_OPEN_AUDIO_DIR: EnsureDir(g_set.dirAudio); ShellOpen(g_set.dirAudio); break;
    case IDM_EXIT: PostMessageW(g_hwnd, WM_CLOSE, 0, 0); break;
    case IDM_SELECT_ALL: SendMessageW(g_list, LB_SETSEL, TRUE, -1); break;
    case IDM_DELETE: DeleteItems(ids, false, false); break;
    case IDM_DELETE_FILE: DeleteItems(ids, false, true); break;
    case IDM_DELETE_ALL: DeleteAllItems(false); break;
    case IDM_DELETE_ALL_FILE: DeleteAllItems(true); break;
    case IDM_CLEAR_DONE: {
        std::vector<int> done;
        for (auto& up : g_items)
            if (up->st == St::Done) done.push_back(up->id);
        for (int d : done) RemoveItem(d);
        RebuildView();
        ScheduleSave();
        break;
    }
    case IDM_SETTINGS: OpenSettings(); break;
    case IDM_UPDATE_YTDLP: RunUpdate(false); break;
    case IDM_RETRY_ALL: {
        std::vector<int> r;
        for (auto& up : g_items)
            if (up->st == St::Error || up->st == St::Canceled) r.push_back(up->id);
        RetryItems(r);
        break;
    }
    case IDM_STOP_ALL: {
        std::vector<int> r;
        for (auto& up : g_items)
            if (up->st == St::Queued || up->st == St::Downloading || up->st == St::Processing) r.push_back(up->id);
        CancelItems(r);
        break;
    }
    case IDM_ABOUT: ShowAbout(); break;
    case IDM_APP_UPDATE: CheckAppUpdate(); break;
    case IDM_PLAY: OpenSelected(); break;
    case IDM_SHOW_FOLDER:
        RevealItem(first);
        break;
    case IDM_COPY_LINK: {
        std::wstring s;
        for (int i : ids) {
            Item* it = FindItem(i);
            if (!it) continue;
            if (!s.empty()) s += L"\r\n";
            s += it->url;
        }
        if (!s.empty()) SetClipboardText(g_hwnd, s);
        break;
    }
    case IDM_OPEN_LINK:
        if (first) ShellOpen(first->url);
        break;
    case IDM_DL_PLAYLIST:
        if (first) AddLink(first->groupType == GROUP_PLAYLIST ? first->groupUrl : first->playlistUrl, MODE_DEFAULT, first->opt.kind);
        break;
    case IDM_DL_CHANNEL:
        if (first) AddLink(first->channelUrl, MODE_DEFAULT, first->opt.kind);
        break;
    case IDM_TO_MP3:
        if (first) ExtractMp3(first);
        break;
    case IDM_REDOWNLOAD:
        if (first) AddLink(first->url, MODE_DIALOG, -1);
        break;
    case IDM_RETRY: RetryItems(ids); break;
    case IDM_CANCEL: CancelItems(ids); break;
    default:
        if (id >= IDM_TAB0 && id < IDM_TAB0 + 5) SetTab(id - IDM_TAB0);
        else if (id >= IDM_SORT0 && id < IDM_SORT0 + 5) {
            g_set.sort = id - IDM_SORT0;
            SendMessageW(g_cbSort, CB_SETCURSEL, g_set.sort, 0);
            RebuildView();
            SaveSettings();
        }
        break;
    }
}

// ====================================================================== window

void SaveWindowPlacement() {
    WINDOWPLACEMENT wp{sizeof wp};
    if (!GetWindowPlacement(g_hwnd, &wp)) return;
    RECT r = wp.rcNormalPosition;
    g_set.hasPos = true;
    g_set.x = r.left;
    g_set.y = r.top;
    g_set.w = r.right - r.left;
    g_set.h = r.bottom - r.top;
    g_set.maximized = wp.showCmd == SW_SHOWMAXIMIZED;
}

HWND MakeCombo(HWND parent, int id) {
    HWND c = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, 0, 0, S(100),
                             S(300), parent, (HMENU)(INT_PTR)id, g_inst, nullptr);
    SendMessageW(c, WM_SETFONT, (WPARAM)g_font, FALSE);
    ThemeControl(c, L"combo");
    return c;
}

HWND MakeEdit(HWND parent, int id, HFONT font, const wchar_t* cue) {
    HWND e = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 10, 10, parent,
                             (HMENU)(INT_PTR)id, g_inst, nullptr);
    SendMessageW(e, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(e, EM_SETCUEBANNER, TRUE, (LPARAM)cue);
    SendMessageW(e, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(0, 0));
    ThemeControl(e, nullptr);
    return e;
}

void AddTool(HWND tool, int sid, bool isRect) {
    TTTOOLINFOW ti{};
    ti.cbSize = sizeof ti;
    ti.hwnd = g_hwnd;
    ti.lpszText = (LPWSTR)T(sid);
    if (isRect) {
        ti.uFlags = TTF_SUBCLASS;
        ti.uId = 1;
        ti.rect = g_rcToggle;
    } else {
        ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
        ti.uId = (UINT_PTR)tool;
    }
    SendMessageW(g_tip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
}

void OnCreate(HWND hwnd) {
    g_hwnd = hwnd;
    g_btnPaste = MakeButton(hwnd, IDC_PASTE, T(S_BTN_PASTE), ico::Paste, BTN_PRIMARY, true);
    g_cbKind = MakeCombo(hwnd, IDC_KIND);
    g_cbQuality = MakeCombo(hwnd, IDC_QUALITY);
    g_cbFormat = MakeCombo(hwnd, IDC_FORMAT);
    g_cbSub = MakeCombo(hwnd, IDC_SUB);
    g_cbAudio = MakeCombo(hwnd, IDC_AUDIO);
    g_cbDir = MakeCombo(hwnd, IDC_DIR);
    g_btnOpenDir = MakeButton(hwnd, IDC_OPENDIR, L"", ico::Folder, BTN_SECONDARY, true);
    g_btnSettings = MakeButton(hwnd, IDC_SETTINGS, L"", ico::Settings, BTN_SECONDARY, true);
    g_filter = MakeEdit(hwnd, IDC_FILTER, g_font, T(S_FILTER_CUE));
    g_cbSort = MakeCombo(hwnd, IDC_SORT);
    g_list = CreateWindowExW(0, L"LISTBOX", nullptr,
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT |
                                 LBS_EXTENDEDSEL | LBS_NOTIFY,
                             0, 0, 10, 10, hwnd, (HMENU)(INT_PTR)IDC_LIST, g_inst, nullptr);
    ThemeControl(g_list, L"list");
    SetWindowSubclass(g_list, ListProc, 1, 0);

    RECT cr;
    GetWindowRect(g_cbKind, &cr);
    g_comboH = cr.bottom - cr.top;

    g_tip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, nullptr, g_inst, nullptr);
    SendMessageW(g_tip, TTM_SETMAXTIPWIDTH, 0, S(360));
    AddTool(nullptr, S_TIP_QUICK, true);
    AddTool(g_btnPaste, S_TIP_PASTE, false);
    AddTool(g_btnOpenDir, S_TIP_OPENDIR, false);
    AddTool(g_btnSettings, S_TIP_SETTINGS, false);

    FillBar();
    std::vector<std::wstring> sorts;
    for (int i = 0; i < 5; ++i) sorts.push_back(T(kSortName[i]));
    ComboSet(g_cbSort, sorts, g_set.sort);

    LoadHistory(g_items, g_nextId);
    for (auto& up : g_items) g_byId[up->id] = up.get();
    ThemeWindow(hwnd, true);
    Layout();
    RebuildView();
    RunVersionCheck();
    // Checked shortly after the window is up: the first run may have to copy
    // yt-dlp into the user profile, which would otherwise stall startup.
    SetTimer(hwnd, TIMER_AUTOUPDATE, 1200, nullptr);
}

bool HandleKey(MSG& m) {
    if (m.message == WM_SYSCHAR) {
        wchar_t c = UpChar((wchar_t)m.wParam);
        for (int i = 0; i < 5; ++i)
            if (c && c == MenuKey(i)) {
                OpenMenu(i, false);
                return true;
            }
        return false;
    }
    if (m.message != WM_KEYDOWN) return false;
    if (m.hwnd != g_hwnd && !IsChild(g_hwnd, m.hwnd)) return false;
    HWND f = GetFocus();
    bool ctrl = GetKeyState(VK_CONTROL) < 0, shift = GetKeyState(VK_SHIFT) < 0, alt = GetKeyState(VK_MENU) < 0;
    bool inEdit = f == g_filter;
    if (alt) return false;
    switch (m.wParam) {
    case VK_TAB:
        if (!ctrl) {
            if (HWND n = GetNextDlgTabItem(g_hwnd, f, shift)) SetFocus(n);
            return true;
        }
        break;
    case VK_F10:
        if (!ctrl && !shift) {
            OpenMenu(0, false);
            return true;
        }
        break;
    case VK_RETURN:
        if (f == g_list) { OpenSelected(); return true; }
        if (f == g_filter) return true;
        break;
    case VK_ESCAPE:
        if (f == g_filter) { SetWindowTextW(g_filter, L""); return true; }
        break;
    case 'V':
        if (ctrl) { PasteLinks(); return true; }
        break;
    case 'A':
        if (ctrl && !inEdit) { SendMessageW(g_list, LB_SETSEL, TRUE, -1); return true; }
        if (ctrl && inEdit) { SendMessageW(f, EM_SETSEL, 0, -1); return true; }
        break;
    case 'F':
        if (ctrl) { SetFocus(g_filter); SendMessageW(g_filter, EM_SETSEL, 0, -1); return true; }
        break;
    case VK_DELETE:
        if (!inEdit) { DeleteItems(SelectedIds(), false, shift); return true; }
        break;
    case VK_OEM_COMMA:
        if (ctrl) { OpenSettings(); return true; }
        break;
    }
    return false;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == g_msgQuit && g_msgQuit) {
        SaveWindowPlacement();
        SaveSettings();
        SaveHistory(g_items);
        DestroyWindow(hwnd);
        return 0;
    }
    if (msg == g_msgTaskbarCreated && g_msgTaskbarCreated) {
        if (!g_taskbar && SUCCEEDED(CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_taskbar))))
            g_taskbar->HrInit();
        UpdateTaskbar();
        return 0;
    }
    switch (msg) {
    case WM_CREATE:
        OnCreate(hwnd);
        return 0;
    case WM_SIZE:
        if (wp != SIZE_MINIMIZED) Layout();
        return 0;
    case WM_GETMINMAXINFO: {
        auto* mm = (MINMAXINFO*)lp;
        mm->ptMinTrackSize.x = S(960);
        mm->ptMinTrackSize.y = S(620);
        return 0;
    }
    case WM_SETTINGCHANGE:
        if (g_set.theme == THEME_SYSTEM && lp && wcscmp((const wchar_t*)lp, L"ImmersiveColorSet") == 0) ApplyAppearance(false);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        PaintMain(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_CTLCOLOREDIT:
        return (LRESULT)(INT_PTR)DlgColor((HDC)wp);
    case WM_CTLCOLORLISTBOX:  // also used by the drop-down part of every combo box
        return (LRESULT)(INT_PTR)DlgColor((HDC)wp, clr.text, clr.bg);
    case WM_MEASUREITEM: {
        auto* mi = (MEASUREITEMSTRUCT*)lp;
        if (mi->CtlType == ODT_LISTBOX) {
            mi->itemHeight = S(78);
            return TRUE;
        }
        break;
    }
    case WM_DRAWITEM: {
        auto* di = (const DRAWITEMSTRUCT*)lp;
        if (di->CtlID == IDC_LIST) {
            DrawListItem(di);
            return TRUE;
        }
        if (DrawFancyButton(di)) return TRUE;
        break;
    }
    case WM_SETCURSOR:
        if ((HWND)wp == hwnd && LOWORD(lp) == HTCLIENT) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);
            bool hand = PtInRect(&g_rcToggle, pt);
            for (auto& r : g_rcTab) hand = hand || PtInRect(&r, pt);
            for (auto& r : g_rcMenu) hand = hand || PtInRect(&r, pt);
            if (hand) {
                SetCursor(LoadCursorW(nullptr, IDC_HAND));
                return TRUE;
            }
            if (PtInRect(&g_rcFilter, pt)) {
                SetCursor(LoadCursorW(nullptr, IDC_IBEAM));
                return TRUE;
            }
        }
        break;
    case WM_MOUSEMOVE: {
        POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        int hot = -1;
        for (int i = 0; i < 5; ++i)
            if (PtInRect(&g_rcMenu[i], pt)) hot = i;
        if (hot != g_menuHover) {
            g_menuHover = hot;
            InvalidateRect(hwnd, &g_rcMenuBar, FALSE);
            // Resting on a title opens it without a click, the way the menu
            // bars in most Windows programs behave once you are in them.
            if (hot >= 0 && g_menuOpen < 0) SetTimer(hwnd, TIMER_MENUHOVER, kMenuHoverDelay, nullptr);
            else KillTimer(hwnd, TIMER_MENUHOVER);
        }
        if (!g_menuTracking) {
            TRACKMOUSEEVENT t{sizeof t, TME_LEAVE, hwnd, 0};
            TrackMouseEvent(&t);
            g_menuTracking = true;
        }
        return 0;
    }
    case WM_MOUSELEAVE:
        g_menuTracking = false;
        KillTimer(hwnd, TIMER_MENUHOVER);
        if (g_menuHover >= 0) {
            g_menuHover = -1;
            InvalidateRect(hwnd, &g_rcMenuBar, FALSE);
        }
        return 0;
    case WM_LBUTTONDOWN: {
        POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        for (int i = 0; i < 5; ++i)
            if (PtInRect(&g_rcMenu[i], pt)) {
                OpenMenu(i, true);
                return 0;
            }
        if (PtInRect(&g_rcToggle, pt)) {
            g_set.quick = !g_set.quick;
            InvalidateRect(hwnd, &g_rcToggle, FALSE);
            SaveSettings();
            return 0;
        }
        for (int i = 0; i < 5; ++i)
            if (PtInRect(&g_rcTab[i], pt)) {
                SetTab(i);
                return 0;
            }
        if (PtInRect(&g_rcFilter, pt)) SetFocus(g_filter);
        return 0;
    }
    case WM_CONTEXTMENU:
        if ((HWND)wp == g_list) {
            POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            if (pt.x == -1 && pt.y == -1) {
                int caret = (int)SendMessageW(g_list, LB_GETCARETINDEX, 0, 0);
                RECT ir{};
                if (caret >= 0) SendMessageW(g_list, LB_GETITEMRECT, caret, (LPARAM)&ir);
                pt = {ir.left + S(140), ir.bottom};
                ClientToScreen(g_list, &pt);
            } else {
                POINT cp = pt;
                ScreenToClient(g_list, &cp);
                int row = HitRow(g_list, cp);
                if (row >= 0) {
                    std::vector<int> sel = SelectedIds();
                    if (std::find(sel.begin(), sel.end(), g_view[row]) == sel.end()) SelectOnlyRow(row);
                } else {
                    SendMessageW(g_list, LB_SETSEL, FALSE, -1);
                }
            }
            ShowContextMenu(pt);
            return 0;
        }
        break;
    case WM_COMMAND: {
        int id = LOWORD(wp), code = HIWORD(wp);
        switch (id) {
        case IDC_PASTE:
            if (code == BN_CLICKED) PasteLinks();
            return 0;
        case IDC_FILTER:
            if (code == EN_CHANGE) RebuildView();
            if (code == EN_SETFOCUS || code == EN_KILLFOCUS) {
                g_filterFocus = code == EN_SETFOCUS;
                RECT r = g_rcFilter;
                InflateRect(&r, 2, 2);
                InvalidateRect(hwnd, &r, FALSE);
            }
            return 0;
        case IDC_KIND:
            if (code == CBN_SELCHANGE) {
                g_set.kind = std::max(0, ComboGet(g_cbKind));
                FillBar();
                SaveSettings();
            }
            return 0;
        case IDC_QUALITY:
            if (code == CBN_SELCHANGE) {
                int s = std::max(0, ComboGet(g_cbQuality));
                if (g_set.kind == KIND_VIDEO) g_set.vq = s;
                else if (std::wstring(kAudioFormat[g_set.afmt].value) == L"mp3") g_set.aq = s;
                SaveSettings();
            }
            return 0;
        case IDC_FORMAT:
            if (code == CBN_SELCHANGE) {
                int s = std::max(0, ComboGet(g_cbFormat));
                if (g_set.kind == KIND_VIDEO) g_set.vfmt = s;
                else {
                    g_set.afmt = s;
                    FillBar();
                }
                SaveSettings();
            }
            return 0;
        case IDC_SUB:
            if (code == CBN_SELCHANGE) {
                g_set.subLang = SubtitleCodeAt(ComboGet(g_cbSub));
                SaveSettings();
            }
            return 0;
        case IDC_AUDIO:
            if (code == CBN_SELCHANGE) {
                g_set.audioLang = AudioLangCodeAt(ComboGet(g_cbAudio));
                SaveSettings();
            }
            return 0;
        case IDC_DIR:
            if ((code == CBN_SELCHANGE && !SendMessageW(g_cbDir, CB_GETDROPPEDSTATE, 0, 0)) || code == CBN_CLOSEUP)
                ApplyDirChoice(ComboGet(g_cbDir));
            return 0;
        case IDC_OPENDIR: {
            std::wstring& dir = g_set.kind == KIND_VIDEO ? g_set.dirVideo : g_set.dirAudio;
            std::wstring p = dir;
            if (PickFolder(hwnd, p, T(S_PICK_FOLDER))) {
                dir = p;
                FillDirCombo();
                SaveSettings();
            }
            return 0;
        }
        case IDC_SETTINGS:
            OpenSettings();
            return 0;
        case IDC_SORT:
            if (code == CBN_SELCHANGE) {
                g_set.sort = std::max(0, ComboGet(g_cbSort));
                RebuildView();
                SaveSettings();
            }
            return 0;
        case IDC_LIST:
            if (code == LBN_DBLCLK) OpenSelected();
            return 0;
        }
        if (lp == 0) OnMenuCommand(id);
        return 0;
    }
    case WM_APP_PROC_LINE: {
        std::unique_ptr<ProcLine> pl((ProcLine*)lp);
        auto d = g_dlProc.find(pl->id);
        if (d != g_dlProc.end())
            if (Item* it = FindItem(d->second)) OnDownloadLine(it, pl->text);
        return 0;
    }
    case WM_APP_PROC_EXIT: {
        std::unique_ptr<ProcExit> pe((ProcExit*)lp);
        if (pe->id == g_versionProc) {
            g_versionProc = 0;
            g_ytdlpVer = Trim(Split(W(pe->output), L'\n')[0]);
            if (g_ytdlpVer.empty() || g_ytdlpVer.size() > 40) g_ytdlpVer = L"?";
            InvalidateRect(hwnd, &g_rcStatus, FALSE);
            return 0;
        }
        if (pe->id == g_updateProc) {
            g_updateProc = 0;
            OnUpdateExit(pe.get());
            return 0;
        }
        auto a = g_analyze.find(pe->id);
        if (a != g_analyze.end()) {
            Analysis an = a->second;
            g_analyze.erase(a);
            OnAnalyzeExit(an, pe.get());
            UpdateStatus();
            return 0;
        }
        auto d = g_dlProc.find(pe->id);
        if (d != g_dlProc.end()) {
            int itemId = d->second;
            g_dlProc.erase(d);
            OnDownloadExit(itemId, pe.get());
            UpdateStatus();
            return 0;
        }
        auto c = g_convProc.find(pe->id);
        if (c != g_convProc.end()) {
            int itemId = c->second;
            g_convProc.erase(c);
            OnConvertExit(itemId, pe.get());
            return 0;
        }
        return 0;
    }
    case WM_APP_THUMB: {
        HBITMAP hb = (HBITMAP)lp;
        Item* it = FindItem((int)wp);
        if (!it) {
            if (hb) DeleteObject(hb);
        } else if (hb) {
            if (it->thumb) DeleteObject(it->thumb);
            it->thumb = hb;
            InvalidateItem(it->id);
        }
        return 0;
    }
    case WM_APP_NEXT_DIALOG:
        PumpDialogs();
        return 0;
    case WM_APP_UPDATE_CHECKED: {
        std::unique_ptr<UpdateInfo> info((UpdateInfo*)lp);
        if (wp && info) g_upd = *info;
        OnUpdateChecked(wp != 0);
        return 0;
    }
    case WM_APP_UPDATE_PROGRESS:
        if (g_updState == UPD_DOWNLOADING) SetUpdateMsg(Fmt(T(S_UPD_DOWNLOADING), (int)wp));
        return 0;
    case WM_APP_UPDATE_DONE: {
        std::unique_ptr<std::wstring> path((std::wstring*)lp);
        g_updState = UPD_IDLE;
        SetUpdateMsg(L"");
        if (!wp || !path || path->empty()) {
            if (!g_updCancel) MessageBoxW(GetLastActivePopup(hwnd), T(S_UPD_DL_FAILED), APP_NAME, MB_ICONWARNING);
            return 0;
        }
        g_upd.file = *path;
        OnUpdateReady();
        return 0;
    }
    case WM_APP_TRAY:
        switch (LOWORD(lp)) {
        case NIN_BALLOONUSERCLICK:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_CONTEXTMENU:
            if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
            break;
        }
        return 0;
    case WM_COPYDATA: {
        auto* cds = (COPYDATASTRUCT*)lp;
        if (cds->dwData == COPYDATA_LINKS && cds->lpData) {
            std::wstring s((const wchar_t*)cds->lpData, cds->cbData / sizeof(wchar_t));
            for (auto& l : ExtractLinks(s)) AddLink(l, MODE_DEFAULT, -1);
        }
        return TRUE;
    }
    case WM_TIMER:
        if (wp == TIMER_SAVE) {
            KillTimer(hwnd, TIMER_SAVE);
            SaveHistory(g_items);
        } else if (wp == TIMER_HINT) {
            KillTimer(hwnd, TIMER_HINT);
            if (!g_hint.empty()) {
                g_hint.clear();
                InvalidateRect(hwnd, &g_rcHint, FALSE);
            }
        } else if (wp == TIMER_AUTOUPDATE) {
            KillTimer(hwnd, TIMER_AUTOUPDATE);
            MaybeAutoUpdate();
        } else if (wp == TIMER_MENUHOVER) {
            KillTimer(hwnd, TIMER_MENUHOVER);
            if (g_menuHover >= 0 && g_menuOpen < 0 && GetForegroundWindow() == hwnd) OpenMenu(g_menuHover, true);
        } else if (wp == TIMER_MENUWATCH) {
            if (g_menuOpen >= 0 && MenuShouldYield()) EndMenu();
        }
        return 0;
    case WM_CLOSE: {
        int busy = 0;
        for (auto& up : g_items)
            if (up->st == St::Downloading || up->st == St::Processing || up->st == St::Queued || up->st == St::Analyzing) ++busy;
        if (busy) {
            TASKDIALOGCONFIG tc{};
            tc.cbSize = sizeof tc;
            tc.hwndParent = hwnd;
            tc.hInstance = g_inst;
            tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
            tc.pszWindowTitle = APP_NAME;
            std::wstring main = Fmt(T(S_MSG_CLOSE_BUSY), busy);
            tc.pszMainInstruction = main.c_str();
            tc.pszContent = T(S_MSG_CLOSE_CONTENT);
            TASKDIALOG_BUTTON btns[] = {{IDOK, T(S_MSG_QUIT)}, {IDCANCEL, T(S_BTN_CANCEL)}};
            tc.pButtons = btns;
            tc.cButtons = 2;
            tc.nDefaultButton = IDCANCEL;
            int btn = 0;
            if (FAILED(TaskDialogIndirect(&tc, &btn, nullptr, nullptr)) || btn != IDOK) return 0;
        }
        g_updCancel = true;  // let a running update download stop on its own
        SaveWindowPlacement();
        SaveSettings();
        SaveHistory(g_items);
        DestroyWindow(hwnd);
        return 0;
    }
    case WM_DESTROY:
        ProcCancelAll();
        if (g_trayAdded) {
            NOTIFYICONDATAW nid{};
            nid.cbSize = sizeof nid;
            nid.hWnd = hwnd;
            nid.uID = 1;
            Shell_NotifyIconW(NIM_DELETE, &nid);
        }
        if (g_taskbar) {
            g_taskbar->Release();
            g_taskbar = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR cmdLine, int show) {
    g_inst = hInst;
    SetEnvironmentVariableW(L"PYTHONIOENCODING", L"utf-8");
    SetEnvironmentVariableW(L"PYTHONUTF8", L"1");

    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"Local\\YouTubeToPC.SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (HWND other = FindWindowW(MAIN_CLASS, nullptr)) {
            std::wstring cmd = cmdLine ? cmdLine : L"";
            COPYDATASTRUCT cds{COPYDATA_LINKS, (DWORD)((cmd.size() + 1) * sizeof(wchar_t)), (PVOID)cmd.c_str()};
            SendMessageW(other, WM_COPYDATA, 0, (LPARAM)&cds);
            if (IsIconic(other)) ShowWindow(other, SW_RESTORE);
            SetForegroundWindow(other);
        }
        return 0;
    }

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    INITCOMMONCONTROLSEX icc{sizeof icc, ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES};
    InitCommonControlsEx(&icc);
    Gdiplus::GdiplusStartupInput gsi;
    ULONG_PTR gdiToken = 0;
    Gdiplus::GdiplusStartup(&gdiToken, &gsi, nullptr);

    LoadSettings();       // sets g_lang
    UiInit();             // fonts follow the language
    ApplyTheme(g_set.theme);
    g_tools = FindTools();
    ThumbStart();
    g_msgTaskbarCreated = RegisterWindowMessageW(L"TaskbarButtonCreated");
    g_msgQuit = RegisterWindowMessageW(L"YouTubeToPC.Quit");

    g_iconBig = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON),
                                  LR_DEFAULTCOLOR);
    g_iconSmall = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
                                    GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof wc;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hIcon = g_iconBig;
    wc.hIconSm = g_iconSmall;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = MAIN_CLASS;
    RegisterClassExW(&wc);

    RECT work;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    int w = std::min<int>(S(1120), work.right - work.left);
    int h = std::min<int>(S(760), work.bottom - work.top);
    int x = work.left + (work.right - work.left - w) / 2;
    int y = work.top + (work.bottom - work.top - h) / 2;
    if (g_set.hasPos && g_set.w > 200 && g_set.h > 200) {
        RECT r{g_set.x, g_set.y, g_set.x + g_set.w, g_set.y + g_set.h};
        MONITORINFO mi{sizeof mi};
        if (GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi)) {
            RECT inter;
            if (IntersectRect(&inter, &r, &mi.rcWork)) {
                x = r.left;
                y = r.top;
                w = r.right - r.left;
                h = r.bottom - r.top;
            }
        }
    }
    HWND hwnd = CreateWindowExW(0, MAIN_CLASS, APP_NAME, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, x, y, w, h, nullptr, nullptr, hInst, nullptr);
    if (!hwnd) return 1;
    ShowWindow(hwnd, g_set.maximized ? SW_SHOWMAXIMIZED : show);
    UpdateWindow(hwnd);
    SetFocus(g_btnPaste);

    if (cmdLine && *cmdLine)
        for (auto& l : ExtractLinks(cmdLine)) AddLink(l, MODE_DEFAULT, -1);

    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        if (HandleKey(m)) continue;
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    Gdiplus::GdiplusShutdown(gdiToken);
    CoUninitialize();
    if (mutex) CloseHandle(mutex);
    return 0;
}
