#pragma once
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <objidl.h>
#include <gdiplus.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <memory>
#include <algorithm>
#include <functional>

#define APP_NAME     L"YouTube to PC"
#define APP_VERSION  L"1.3.3"
#define APP_DIRNAME  L"YouTubeToPC"
#define APP_FOLDER   L"YouTube to PC"
#define MAIN_CLASS   L"YouTubeToPC.Main"
#define IDI_APP      1

enum : UINT {
    WM_APP_PROC_LINE = WM_APP + 1,
    WM_APP_PROC_EXIT,
    WM_APP_THUMB,
    WM_APP_TRAY,
    WM_APP_NEXT_DIALOG,
    WM_APP_UPDATE_CHECKED,   // lp: new UpdateInfo*, wp: 1 on success
    WM_APP_UPDATE_PROGRESS,  // wp: percent
    WM_APP_UPDATE_DONE,      // lp: new std::wstring* with the saved path
};

#include "lang.h"

// ---- theme ----
enum { THEME_SYSTEM = 0, THEME_LIGHT = 1, THEME_DARK = 2 };
struct Palette {
    COLORREF bg, panel, line, border, borderStrong;
    COLORREF text, sub, faint;
    COLORREF accent, accentHover, accentPress, accentSoft, accentMuted;
    COLORREF hover, track, success, danger;
    bool dark;
};
extern Palette clr;
bool SystemPrefersDark();
void ApplyTheme(int mode);               // updates the palette and process-wide dark mode
void ThemeWindow(HWND hwnd, bool main);  // title bar + non-client
void ThemeControl(HWND ctl, const wchar_t* kind);  // kind: "list", "combo", "listview", nullptr

// Glyphs from "Segoe MDL2 Assets" (Windows 10/11)
namespace ico {
constexpr wchar_t Link = 0xE71B;
constexpr wchar_t Paste = 0xE77F;
constexpr wchar_t Download = 0xE896;
constexpr wchar_t Settings = 0xE713;
constexpr wchar_t Search = 0xE721;
constexpr wchar_t Folder = 0xED25;
constexpr wchar_t More = 0xE712;
constexpr wchar_t Check = 0xE73E;
constexpr wchar_t Error = 0xE783;
constexpr wchar_t Clock = 0xE823;
constexpr wchar_t Video = 0xE714;
constexpr wchar_t Music = 0xEC4F;
constexpr wchar_t Pause = 0xE769;
constexpr wchar_t Chevron = 0xE70D;
}  // namespace ico

extern HINSTANCE g_inst;
extern int g_dpi;
inline int S(int v) { return MulDiv(v, g_dpi, 96); }

// ---- strings ----
std::wstring W(const std::string& utf8);
std::string U8(const std::wstring& wide);
std::wstring Fmt(const wchar_t* fmt, ...);
std::wstring Trim(const std::wstring& s);
std::wstring Lower(std::wstring s);
std::wstring Upper(std::wstring s);
bool StartsWith(const std::wstring& s, const std::wstring& prefix);
bool ContainsI(const std::wstring& hay, const std::wstring& needle);
std::vector<std::wstring> Split(const std::wstring& s, wchar_t sep);
std::wstring FormatSize(long long bytes);
std::wstring FormatDuration(double seconds);
std::wstring FormatEta(double seconds);
long long NowMs();
int CompareVersions(const std::wstring& a, const std::wstring& b);  // <0, 0 or >0

// ---- files & paths ----
std::wstring ExeDir();
std::wstring PathJoin(const std::wstring& a, const std::wstring& b);
std::wstring PathDir(const std::wstring& p);
std::wstring PathName(const std::wstring& p);
std::wstring PathStem(const std::wstring& p);
bool FileExists(const std::wstring& p);
bool DirExists(const std::wstring& p);
long long FileSize(const std::wstring& p);
void EnsureDir(const std::wstring& p);
bool DirWritable(const std::wstring& p);
std::wstring KnownFolder(REFKNOWNFOLDERID id);
std::wstring AppDataDir();
std::wstring LocalDataDir();
std::wstring SanitizeFileName(const std::wstring& name);
std::wstring QuoteArg(const std::wstring& a);
std::wstring FindOnPath(const wchar_t* exe);
bool ReadFileBytes(const std::wstring& path, std::string& out);
bool WriteFileBytes(const std::wstring& path, const std::string& data);

// ---- shell ----
bool ShellOpen(const std::wstring& target);
void ShowInFolder(const std::wstring& path);
bool IsProcessElevated();
bool SetClipboardText(HWND owner, const std::wstring& text);
std::wstring GetClipboardText(HWND owner);
bool PickFolder(HWND owner, std::wstring& path, const wchar_t* title);
bool PickFile(HWND owner, std::wstring& path, const wchar_t* title, const wchar_t* filterName, const wchar_t* filterSpec);
bool RecycleFiles(const std::vector<std::wstring>& files);
std::wstring GetText(HWND h);

// ---- network ----
bool HttpGetText(const std::wstring& url, std::string& out);  // small documents only
// progress returns false to abort; it runs on the calling thread
bool DownloadToFile(const std::wstring& url, const std::wstring& dest,
                    const std::function<bool(long long done, long long total)>& progress);

// ---- child processes (yt-dlp, ffmpeg) ----
struct ProcLine { int id; std::wstring text; };
struct ProcExit { int id; DWORD code; bool canceled; std::string output; };
int ProcStart(const std::wstring& exe, const std::wstring& args, HWND notify, bool lineMode);
void ProcCancel(int id);
void ProcCancelAll();

// ---- thumbnails ----
void ThumbStart();
void ThumbRequest(const std::wstring& videoId, HWND notify, WPARAM tag, int w, int h);

// ---- ui helpers ----
extern HFONT g_font, g_fontBold, g_fontSmall, g_fontTitle, g_fontLarge, g_fontHeading;
extern HFONT g_fontIcon, g_fontIconSmall, g_fontIconLarge;
void UiInit();
void UiReloadFonts();
HBRUSH BrushOf(COLORREF c);
HWND Ctl(HWND parent, const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id,
         HFONT font = nullptr, DWORD exStyle = 0);
enum BtnKind { BTN_PRIMARY, BTN_SECONDARY, BTN_GHOST };
HWND MakeButton(HWND parent, int id, const wchar_t* text, wchar_t icon, BtnKind kind, bool onPanel = false, int x = 0, int y = 0,
                int w = 0, int h = 0);
bool DrawFancyButton(const DRAWITEMSTRUCT* d);
void FillSolid(HDC hdc, const RECT& r, COLORREF c);
void FillRound(HDC hdc, const RECT& r, int radius, COLORREF fill, COLORREF border = CLR_INVALID, float borderWidth = 1.0f);
void FillEllipse(HDC hdc, const RECT& r, COLORREF fill);
void DrawTextIn(HDC hdc, const std::wstring& s, RECT r, COLORREF color, HFONT font, UINT flags);
// Same, but & marks the next character for underlining, as in menu titles.
void DrawTextMn(HDC hdc, const std::wstring& s, RECT r, COLORREF color, HFONT font, UINT flags);
std::wstring StripMn(const std::wstring& s);  // drops the & so the text can be measured
int TextWidth(HDC hdc, HFONT font, const std::wstring& s);
int FontHeight(HFONT font);
void ComboSet(HWND cb, const std::vector<std::wstring>& items, int sel);
int ComboGet(HWND cb);
INT_PTR RunDialog(HWND owner, const wchar_t* title, DLGPROC proc, LPARAM param);
void DlgPlace(HWND dlg, int w, int h);
INT_PTR DlgColor(HDC hdc, COLORREF text, COLORREF bg);
inline INT_PTR DlgColor(HDC hdc) { return DlgColor(hdc, clr.text, clr.bg); }
