#include "common.h"

HFONT g_font, g_fontBold, g_fontSmall, g_fontTitle, g_fontLarge, g_fontHeading;
HFONT g_fontIcon, g_fontIconSmall, g_fontIconLarge;
int g_lang = LANG_KO;

Palette clr = {};

// ------------------------------------------------------------ theme

namespace {
// Undocumented uxtheme entry points used by every Win32 app that supports dark mode.
using FnSetPreferredAppMode = int(WINAPI*)(int);
using FnAllowDarkModeForWindow = BOOL(WINAPI*)(HWND, BOOL);
using FnFlushMenuThemes = void(WINAPI*)();
FnSetPreferredAppMode pSetPreferredAppMode = nullptr;
FnAllowDarkModeForWindow pAllowDarkModeForWindow = nullptr;
FnFlushMenuThemes pFlushMenuThemes = nullptr;
bool g_darkApi = false;

void LoadDarkApi() {
    static bool tried = false;
    if (tried) return;
    tried = true;
    HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
    if (!ux) ux = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!ux) return;
    pSetPreferredAppMode = (FnSetPreferredAppMode)GetProcAddress(ux, MAKEINTRESOURCEA(135));
    pAllowDarkModeForWindow = (FnAllowDarkModeForWindow)GetProcAddress(ux, MAKEINTRESOURCEA(133));
    pFlushMenuThemes = (FnFlushMenuThemes)GetProcAddress(ux, MAKEINTRESOURCEA(136));
    g_darkApi = pSetPreferredAppMode && pAllowDarkModeForWindow;
}

const Palette kLight = {
    RGB(255, 255, 255), RGB(246, 247, 249), RGB(232, 234, 238), RGB(214, 218, 225), RGB(189, 195, 205),
    RGB(28, 32, 38),    RGB(100, 108, 120), RGB(150, 157, 168),
    RGB(37, 99, 235),   RGB(29, 78, 216),   RGB(30, 64, 175),   RGB(234, 241, 254), RGB(147, 180, 245),
    RGB(245, 247, 250), RGB(228, 231, 236), RGB(22, 150, 74),   RGB(214, 40, 40),
    false,
};
const Palette kDark = {
    RGB(32, 32, 32),    RGB(43, 43, 43),    RGB(58, 58, 58),    RGB(72, 72, 72),    RGB(100, 100, 100),
    RGB(240, 240, 240), RGB(168, 174, 182), RGB(128, 134, 143),
    RGB(77, 144, 254),  RGB(106, 163, 255), RGB(58, 118, 220),  RGB(38, 53, 76),    RGB(60, 94, 150),
    RGB(46, 46, 46),    RGB(64, 64, 64),    RGB(72, 199, 116),  RGB(255, 112, 112),
    true,
};
}  // namespace

bool SystemPrefersDark() {
    DWORD v = 1, size = sizeof v;
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                     L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS)
        return false;
    return v == 0;
}

void ApplyTheme(int mode) {
    bool dark = mode == THEME_DARK || (mode == THEME_SYSTEM && SystemPrefersDark());
    clr = dark ? kDark : kLight;
    LoadDarkApi();
    if (pSetPreferredAppMode) pSetPreferredAppMode(dark ? 2 /*ForceDark*/ : 3 /*ForceLight*/);
    if (pFlushMenuThemes) pFlushMenuThemes();
}

void ThemeWindow(HWND hwnd, bool main) {
    if (!hwnd) return;
    LoadDarkApi();
    if (pAllowDarkModeForWindow) pAllowDarkModeForWindow(hwnd, clr.dark);
    BOOL dark = clr.dark;
    if (FAILED(DwmSetWindowAttribute(hwnd, 20 /*DWMWA_USE_IMMERSIVE_DARK_MODE*/, &dark, sizeof dark)))
        DwmSetWindowAttribute(hwnd, 19, &dark, sizeof dark);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    if (main && pFlushMenuThemes) pFlushMenuThemes();
}

void ThemeControl(HWND ctl, const wchar_t* kind) {
    if (!ctl) return;
    LoadDarkApi();
    if (pAllowDarkModeForWindow) pAllowDarkModeForWindow(ctl, clr.dark);
    std::wstring k = kind ? kind : L"";
    if (k == L"combo") {
        SetWindowTheme(ctl, clr.dark ? L"DarkMode_CFD" : L"CFD", nullptr);
    } else if (k == L"button") {
        // Themed check boxes and radio buttons ignore WM_CTLCOLORSTATIC and use the
        // theme's own text colour, so they need the dark variant of the Button class.
        SetWindowTheme(ctl, clr.dark ? L"DarkMode_Explorer" : nullptr, nullptr);
    } else if (k == L"list" || k == L"listview") {
        SetWindowTheme(ctl, clr.dark ? L"DarkMode_Explorer" : L"Explorer", nullptr);
        if (k == L"listview") {
            ListView_SetBkColor(ctl, clr.bg);
            ListView_SetTextBkColor(ctl, clr.bg);
            ListView_SetTextColor(ctl, clr.text);
        }
    } else {
        SetWindowTheme(ctl, clr.dark ? L"DarkMode_Explorer" : nullptr, nullptr);
    }
    InvalidateRect(ctl, nullptr, TRUE);
}

// ------------------------------------------------------------ fonts

static HFONT MakeFont(const wchar_t* face, int tenthPt, int weight) {
    LOGFONTW lf{};
    lf.lfHeight = -MulDiv(tenthPt, g_dpi, 720);
    lf.lfWeight = weight;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = CLEARTYPE_QUALITY;
    lf.lfOutPrecision = OUT_TT_PRECIS;
    wcsncpy(lf.lfFaceName, face, LF_FACESIZE - 1);
    return CreateFontIndirectW(&lf);
}

void UiReloadFonts() {
    for (HFONT* f : {&g_font, &g_fontBold, &g_fontSmall, &g_fontTitle, &g_fontLarge, &g_fontHeading})
        if (*f) {
            DeleteObject(*f);
            *f = nullptr;
        }
    const wchar_t* face = kLangs[(g_lang >= 0 && g_lang < LANG_COUNT) ? g_lang : LANG_KO].font;
    g_font = MakeFont(face, 90, FW_NORMAL);
    g_fontBold = MakeFont(face, 90, FW_BOLD);
    g_fontSmall = MakeFont(face, 80, FW_NORMAL);
    g_fontTitle = MakeFont(face, 100, FW_BOLD);
    g_fontLarge = MakeFont(face, 105, FW_NORMAL);
    g_fontHeading = MakeFont(face, 135, FW_BOLD);
}

void UiInit() {
    HDC hdc = GetDC(nullptr);
    g_dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(nullptr, hdc);
    UiReloadFonts();
    const wchar_t* icons = L"Segoe MDL2 Assets";
    g_fontIcon = MakeFont(icons, 110, FW_NORMAL);
    g_fontIconSmall = MakeFont(icons, 85, FW_NORMAL);
    g_fontIconLarge = MakeFont(icons, 280, FW_NORMAL);
}

HBRUSH BrushOf(COLORREF c) {
    static std::map<COLORREF, HBRUSH> cache;
    auto it = cache.find(c);
    if (it != cache.end()) return it->second;
    HBRUSH b = CreateSolidBrush(c);
    cache[c] = b;
    return b;
}

HWND Ctl(HWND parent, const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id,
         HFONT font, DWORD exStyle) {
    HWND c = CreateWindowExW(exStyle, cls, text, WS_CHILD | WS_VISIBLE | style, S(x), S(y), S(w), S(h), parent,
                             (HMENU)(INT_PTR)id, g_inst, nullptr);
    SendMessageW(c, WM_SETFONT, (WPARAM)(font ? font : g_font), FALSE);
    std::wstring k = cls;
    if (k == WC_COMBOBOXW) ThemeControl(c, L"combo");
    else if (k == WC_LISTVIEWW) ThemeControl(c, L"listview");
    else if (k == L"LISTBOX") ThemeControl(c, L"list");
    else if (k == L"EDIT") ThemeControl(c, nullptr);
    else if (k == L"BUTTON") ThemeControl(c, L"button");
    return c;
}

// ------------------------------------------------------------ drawing

static Gdiplus::Color GC(COLORREF c, BYTE a = 255) {
    return Gdiplus::Color(a, GetRValue(c), GetGValue(c), GetBValue(c));
}

static void RoundPath(Gdiplus::GraphicsPath& p, float x, float y, float w, float h, float r) {
    float d = std::min(r * 2, std::min(w, h));
    if (d <= 0.5f) {
        p.AddRectangle(Gdiplus::RectF(x, y, w, h));
        return;
    }
    p.AddArc(x, y, d, d, 180, 90);
    p.AddArc(x + w - d, y, d, d, 270, 90);
    p.AddArc(x + w - d, y + h - d, d, d, 0, 90);
    p.AddArc(x, y + h - d, d, d, 90, 90);
    p.CloseFigure();
}

void FillSolid(HDC hdc, const RECT& r, COLORREF c) {
    SetBkColor(hdc, c);
    ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, nullptr, 0, nullptr);
}

void FillRound(HDC hdc, const RECT& r, int radius, COLORREF fill, COLORREF border, float bw) {
    Gdiplus::Graphics g(hdc);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    float x = (float)r.left, y = (float)r.top, w = (float)(r.right - r.left), h = (float)(r.bottom - r.top);
    if (fill != CLR_INVALID) {
        Gdiplus::GraphicsPath p;
        RoundPath(p, x, y, w, h, (float)radius);
        Gdiplus::SolidBrush b(GC(fill));
        g.FillPath(&b, &p);
    }
    if (border != CLR_INVALID) {
        Gdiplus::GraphicsPath p;
        float in = bw / 2;
        RoundPath(p, x + in, y + in, w - bw, h - bw, std::max(0.0f, radius - in));
        Gdiplus::Pen pen(GC(border), bw);
        g.DrawPath(&pen, &p);
    }
}

void FillEllipse(HDC hdc, const RECT& r, COLORREF fill) {
    Gdiplus::Graphics g(hdc);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    Gdiplus::SolidBrush b(GC(fill));
    g.FillEllipse(&b, (float)r.left, (float)r.top, (float)(r.right - r.left), (float)(r.bottom - r.top));
}

void DrawTextIn(HDC hdc, const std::wstring& s, RECT r, COLORREF color, HFONT font, UINT flags) {
    HGDIOBJ old = SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, s.c_str(), (int)s.size(), &r, flags | DT_NOPREFIX);
    SelectObject(hdc, old);
}

void DrawTextMn(HDC hdc, const std::wstring& s, RECT r, COLORREF color, HFONT font, UINT flags) {
    HGDIOBJ old = SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, s.c_str(), (int)s.size(), &r, flags);  // no DT_NOPREFIX: & underlines
    SelectObject(hdc, old);
}

std::wstring StripMn(const std::wstring& s) {
    std::wstring r;
    r.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == L'&' && i + 1 < s.size() && s[i + 1] == L'&') { r += L'&'; ++i; continue; }
        if (s[i] == L'&') continue;
        r += s[i];
    }
    return r;
}

int TextWidth(HDC hdc, HFONT font, const std::wstring& s) {
    HGDIOBJ old = SelectObject(hdc, font);
    SIZE sz{};
    GetTextExtentPoint32W(hdc, s.c_str(), (int)s.size(), &sz);
    SelectObject(hdc, old);
    return sz.cx;
}

int FontHeight(HFONT font) {
    HDC hdc = GetDC(nullptr);
    HGDIOBJ old = SelectObject(hdc, font);
    TEXTMETRICW tm{};
    GetTextMetricsW(hdc, &tm);
    SelectObject(hdc, old);
    ReleaseDC(nullptr, hdc);
    return tm.tmHeight;
}

// ------------------------------------------------------------ buttons

namespace {
struct BtnInfo {
    BtnKind kind;
    wchar_t icon;
    bool onPanel;
    bool hover;
    bool tracking;
};

LRESULT CALLBACK BtnProc(HWND h, UINT m, WPARAM w, LPARAM l, UINT_PTR, DWORD_PTR) {
    auto* bi = (BtnInfo*)GetWindowLongPtrW(h, GWLP_USERDATA);
    switch (m) {
    case WM_MOUSEMOVE:
        if (bi && !bi->hover) {
            bi->hover = true;
            InvalidateRect(h, nullptr, FALSE);
        }
        if (bi && !bi->tracking) {
            TRACKMOUSEEVENT t{sizeof t, TME_LEAVE, h, 0};
            TrackMouseEvent(&t);
            bi->tracking = true;
        }
        break;
    case WM_MOUSELEAVE:
        if (bi) {
            bi->hover = false;
            bi->tracking = false;
            InvalidateRect(h, nullptr, FALSE);
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_LBUTTONDBLCLK:
        SendMessageW(h, WM_LBUTTONDOWN, w, l);
        return 0;
    case WM_NCDESTROY:
        delete bi;
        SetWindowLongPtrW(h, GWLP_USERDATA, 0);
        RemoveWindowSubclass(h, BtnProc, 1);
        break;
    }
    return DefSubclassProc(h, m, w, l);
}
}  // namespace

HWND MakeButton(HWND parent, int id, const wchar_t* text, wchar_t icon, BtnKind kind, bool onPanel, int x, int y, int w, int h) {
    HWND b = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW, S(x), S(y), S(w), S(h),
                             parent, (HMENU)(INT_PTR)id, g_inst, nullptr);
    auto* bi = new BtnInfo{kind, icon, onPanel, false, false};
    SetWindowLongPtrW(b, GWLP_USERDATA, (LONG_PTR)bi);
    SetWindowSubclass(b, BtnProc, 1, 0);
    SendMessageW(b, WM_SETFONT, (WPARAM)(kind == BTN_PRIMARY ? g_fontBold : g_font), FALSE);
    return b;
}

bool DrawFancyButton(const DRAWITEMSTRUCT* d) {
    if (d->CtlType != ODT_BUTTON) return false;
    auto* bi = (BtnInfo*)GetWindowLongPtrW(d->hwndItem, GWLP_USERDATA);
    if (!bi) return false;
    RECT rc = d->rcItem;
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    HDC mem = CreateCompatibleDC(d->hDC);
    HBITMAP bmp = CreateCompatibleBitmap(d->hDC, w, h);
    HGDIOBJ ob = SelectObject(mem, bmp);
    RECT r{0, 0, w, h};
    FillSolid(mem, r, bi->onPanel ? clr.panel : clr.bg);

    bool pressed = d->itemState & ODS_SELECTED;
    bool disabled = d->itemState & ODS_DISABLED;
    bool focus = (d->itemState & ODS_FOCUS) && !(d->itemState & ODS_NOFOCUSRECT);
    COLORREF fill, border = CLR_INVALID, fg;
    switch (bi->kind) {
    case BTN_PRIMARY:
        fill = disabled ? clr.accentMuted : pressed ? clr.accentPress : bi->hover ? clr.accentHover : clr.accent;
        fg = RGB(255, 255, 255);
        break;
    case BTN_SECONDARY:
        fill = pressed ? clr.track : bi->hover ? clr.hover : clr.bg;
        border = clr.borderStrong;
        fg = disabled ? clr.faint : clr.text;
        break;
    default:
        fill = pressed ? clr.track : bi->hover ? clr.hover : (bi->onPanel ? clr.panel : clr.bg);
        fg = disabled ? clr.faint : clr.sub;
        break;
    }
    FillRound(mem, r, S(6), fill, border);
    if (focus) {
        RECT fr = r;
        if (bi->kind == BTN_PRIMARY) {
            InflateRect(&fr, -S(3), -S(3));
            FillRound(mem, fr, S(4), CLR_INVALID, RGB(255, 255, 255), 1.2f);
        } else {
            FillRound(mem, fr, S(6), CLR_INVALID, clr.accent, 2.0f);
        }
    }

    std::wstring text = GetText(d->hwndItem);
    HFONT font = (HFONT)SendMessageW(d->hwndItem, WM_GETFONT, 0, 0);
    if (!font) font = g_font;
    std::wstring icon = bi->icon ? std::wstring(1, bi->icon) : L"";
    HFONT iconFont = g_fontIconSmall;
    int iw = icon.empty() ? 0 : TextWidth(mem, iconFont, icon);
    int tw = text.empty() ? 0 : TextWidth(mem, font, text);
    int gap = (iw && tw) ? S(8) : 0;
    int x = (w - (iw + gap + tw)) / 2;
    int dy = pressed ? 1 : 0;
    if (iw) {
        RECT ir{x, dy, x + iw, h + dy};
        DrawTextIn(mem, icon, ir, fg, iconFont, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
    if (tw) {
        RECT tr{x + iw + gap, dy, x + iw + gap + tw + 2, h + dy};
        DrawTextIn(mem, text, tr, fg, font, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
    BitBlt(d->hDC, rc.left, rc.top, w, h, mem, 0, 0, SRCCOPY);
    SelectObject(mem, ob);
    DeleteObject(bmp);
    DeleteDC(mem);
    return true;
}

// ------------------------------------------------------------ combos

void ComboSet(HWND cb, const std::vector<std::wstring>& items, int sel) {
    SendMessageW(cb, WM_SETREDRAW, FALSE, 0);
    SendMessageW(cb, CB_RESETCONTENT, 0, 0);
    for (auto& s : items) SendMessageW(cb, CB_ADDSTRING, 0, (LPARAM)s.c_str());
    if (sel < 0 || sel >= (int)items.size()) sel = items.empty() ? -1 : 0;
    SendMessageW(cb, CB_SETCURSEL, sel, 0);
    SendMessageW(cb, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(cb, nullptr, TRUE);
}

int ComboGet(HWND cb) { return (int)SendMessageW(cb, CB_GETCURSEL, 0, 0); }

// ------------------------------------------------------------ dialogs

INT_PTR RunDialog(HWND owner, const wchar_t* title, DLGPROC proc, LPARAM param) {
    std::vector<WORD> t;
    auto dw = [&](DWORD d) {
        t.push_back(LOWORD(d));
        t.push_back(HIWORD(d));
    };
    dw(WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | WS_CLIPCHILDREN);
    dw(0);
    t.push_back(0);
    t.push_back(0);
    t.push_back(0);
    t.push_back(200);
    t.push_back(100);
    t.push_back(0);
    t.push_back(0);
    for (const wchar_t* c = title; *c; ++c) t.push_back((WORD)*c);
    t.push_back(0);
    return DialogBoxIndirectParamW(g_inst, (LPCDLGTEMPLATEW)t.data(), owner, proc, param);
}

void DlgPlace(HWND dlg, int w, int h) {
    RECT r{0, 0, S(w), S(h)};
    AdjustWindowRectEx(&r, (DWORD)GetWindowLongW(dlg, GWL_STYLE), FALSE, (DWORD)GetWindowLongW(dlg, GWL_EXSTYLE));
    int ww = r.right - r.left, wh = r.bottom - r.top;
    HWND owner = GetWindow(dlg, GW_OWNER);
    RECT o{};
    if (owner && IsWindowVisible(owner) && !IsIconic(owner)) GetWindowRect(owner, &o);
    else SystemParametersInfoW(SPI_GETWORKAREA, 0, &o, 0);
    int x = o.left + ((o.right - o.left) - ww) / 2;
    int y = o.top + ((o.bottom - o.top) - wh) / 2;
    MONITORINFO mi{sizeof mi};
    if (GetMonitorInfoW(MonitorFromRect(&o, MONITOR_DEFAULTTONEAREST), &mi)) {
        x = std::max<int>(mi.rcWork.left, std::min<int>(x, mi.rcWork.right - ww));
        y = std::max<int>(mi.rcWork.top, std::min<int>(y, mi.rcWork.bottom - wh));
    }
    SetWindowPos(dlg, nullptr, x, y, ww, wh, SWP_NOZORDER | SWP_NOACTIVATE);
    HICON big = (HICON)LoadImageW(g_inst, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, GetSystemMetrics(SM_CXICON),
                                  GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR | LR_SHARED);
    SendMessageW(dlg, WM_SETICON, ICON_BIG, (LPARAM)big);
    ThemeWindow(dlg, false);
}

INT_PTR DlgColor(HDC hdc, COLORREF text, COLORREF bg) {
    SetTextColor(hdc, text);
    SetBkColor(hdc, bg);
    return (INT_PTR)BrushOf(bg);
}
