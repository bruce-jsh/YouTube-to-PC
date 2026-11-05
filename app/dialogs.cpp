#include "model.h"

// ====================================================================== options dialog

namespace {

enum {
    IDC_O_TITLE = 1001,
    IDC_O_META,
    IDC_O_VIDEO,
    IDC_O_AUDIO,
    IDC_O_QUALITY,
    IDC_O_FORMAT,
    IDC_O_SUB,
    IDC_O_ALANG,
    IDC_O_DIR,
    IDC_O_BROWSE,
    IDC_O_ALL,
    IDC_O_LIST,
};

constexpr int OW = 620;

struct OptDlg {
    VideoMeta* m = nullptr;
    DlOptions* o = nullptr;
    HWND meta = nullptr, cbQ = nullptr, cbF = nullptr, cbS = nullptr, cbA = nullptr, edDir = nullptr;
    HWND chkAll = nullptr, lv = nullptr, ok = nullptr;
    HBITMAP thumb = nullptr;
    int kind = KIND_VIDEO;
    std::wstring vfmt, afmt, dir;
    bool customDir = false;
    std::vector<int> qVals;
    std::vector<std::wstring> aqVals, fmtVals, alVals;
    std::vector<SubTrack> subVals;
    int footerY = 0;
};

void FillQuality(OptDlg* d) {
    std::vector<std::wstring> items;
    int sel = 0;
    d->qVals.clear();
    d->aqVals.clear();
    if (d->kind == KIND_VIDEO) {
        if (!d->m->isGroup && !d->m->qualities.empty()) {
            auto label = [&](const QualityInfo& q) {
                std::wstring s = Fmt(L"%dp", q.height);
                if (q.fps > 30) s += std::to_wstring(q.fps);
                if (q.size > 0) s += Fmt(T(S_OPT_APPROX), FormatSize(q.size + d->m->audioSize).c_str());
                return s;
            };
            items.push_back(Fmt(T(S_OPT_BEST_WITH), label(d->m->qualities[0]).c_str()));
            d->qVals.push_back(0);
            for (auto& q : d->m->qualities) {
                items.push_back(label(q));
                d->qVals.push_back(q.height);
            }
            if (d->o->height > 0) {
                sel = (int)items.size() - 1;
                for (size_t i = 1; i < d->qVals.size(); ++i)
                    if (d->qVals[i] <= d->o->height) {
                        sel = (int)i;
                        break;
                    }
            }
        } else {
            for (auto& c : kVideoQuality) {
                items.push_back(c.label());
                d->qVals.push_back(_wtoi(c.value));
                if (d->qVals.back() == d->o->height) sel = (int)items.size() - 1;
            }
        }
        ComboSet(d->cbQ, items, sel);
        EnableWindow(d->cbQ, TRUE);
        return;
    }
    if (d->afmt == L"mp3") {
        for (auto& c : kAudioQuality) {
            std::wstring v = c.value, s = c.label();
            if (v == L"0") s = T(S_Q_BEST_VBR);
            else if (d->m->duration > 0)
                s += Fmt(T(S_OPT_APPROX), FormatSize((long long)(_wtoi(v.c_str()) * 1000.0 / 8 * d->m->duration)).c_str());
            items.push_back(s);
            d->aqVals.push_back(v);
            if (v == d->o->aq) sel = (int)items.size() - 1;
        }
        ComboSet(d->cbQ, items, sel);
        EnableWindow(d->cbQ, TRUE);
    } else {
        std::wstring s = T(S_OPT_ORIGINAL_Q);
        if (d->m->audioAbr > 0) s += Fmt(L"   %.0f kbps", d->m->audioAbr);
        items.push_back(s);
        d->aqVals.push_back(L"0");
        ComboSet(d->cbQ, items, 0);
        EnableWindow(d->cbQ, FALSE);
    }
}

void FillFormat(OptDlg* d) {
    const auto& list = d->kind == KIND_VIDEO ? kVideoFormat : kAudioFormat;
    const std::wstring& cur = d->kind == KIND_VIDEO ? d->vfmt : d->afmt;
    d->fmtVals.clear();
    std::vector<std::wstring> items;
    int sel = 0;
    for (size_t i = 0; i < list.size(); ++i) {
        d->fmtVals.push_back(list[i].value);
        std::wstring label = list[i].label();
        if (d->kind == KIND_AUDIO && list[i].value != std::wstring(L"mp3")) label = Fmt(T(S_FMT_ORIGINAL), label.c_str());
        items.push_back(label);
        if (cur == list[i].value) sel = (int)i;
    }
    ComboSet(d->cbF, items, sel);
}

void FillSubs(OptDlg* d) {
    d->subVals.clear();
    d->subVals.push_back({L"", T(S_SUB_NONE), false});
    std::vector<std::wstring> items;
    int sel = 0;
    if (d->m->isGroup) {
        for (int i = 0; i < kSubtitleCodeCount; ++i) d->subVals.push_back({kSubtitleCodes[i], NativeLangName(kSubtitleCodes[i]), d->o->subAuto});
        d->subVals.push_back({L"all", T(S_SUB_ALL), d->o->subAuto});
    } else {
        for (auto& s : d->m->subs) d->subVals.push_back(s);
        if (d->subVals.size() == 1) d->subVals[0].label = T(S_OPT_NO_SUBS);
    }
    for (size_t i = 0; i < d->subVals.size(); ++i) {
        items.push_back(d->subVals[i].label);
        if (!sel && !d->o->sub.empty() && d->subVals[i].code == d->o->sub) sel = (int)i;
    }
    ComboSet(d->cbS, items, sel);
    EnableWindow(d->cbS, items.size() > 1);
}

void FillAudioLangs(OptDlg* d) {
    d->alVals = {L""};
    std::vector<std::wstring> items = {T(S_AL_DEFAULT)};
    if (d->m->isGroup) {
        for (int i = 0; i < kSubtitleCodeCount; ++i) {
            d->alVals.push_back(kSubtitleCodes[i]);
            items.push_back(NativeLangName(kSubtitleCodes[i]));
        }
    } else {
        for (auto& a : d->m->audioLangs) {
            d->alVals.push_back(a.first);
            items.push_back(a.second);
        }
        if (items.size() == 1) items[0] = T(S_OPT_ONE_TRACK);
    }
    int sel = 0;
    for (size_t i = 1; i < d->alVals.size(); ++i)
        if (!d->o->alang.empty() && StartsWith(d->alVals[i], d->o->alang)) {
            sel = (int)i;
            break;
        }
    ComboSet(d->cbA, items, sel);
    EnableWindow(d->cbA, items.size() > 1);
}

int CheckedCount(OptDlg* d) {
    if (!d->lv) return 1;
    int n = ListView_GetItemCount(d->lv), c = 0;
    for (int i = 0; i < n; ++i)
        if (ListView_GetCheckState(d->lv, i)) ++c;
    return c;
}

void UpdateOk(OptDlg* d) {
    if (!d->lv) return;
    int c = CheckedCount(d);
    SetWindowTextW(d->ok, Fmt(T(S_OPT_DOWNLOAD_N), c).c_str());
    EnableWindow(d->ok, c > 0);
    InvalidateRect(d->ok, nullptr, FALSE);
}

void SetKind(OptDlg* d, int kind) {
    d->kind = kind;
    FillQuality(d);
    FillFormat(d);
    if (!d->customDir) {
        d->dir = kind == KIND_VIDEO ? g_set.dirVideo : g_set.dirAudio;
        SetWindowTextW(d->edDir, d->dir.c_str());
    }
}

INT_PTR CALLBACK OptProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp) {
    auto* d = (OptDlg*)GetWindowLongPtrW(dlg, DWLP_USER);
    switch (msg) {
    case WM_INITDIALOG: {
        d = (OptDlg*)lp;
        SetWindowLongPtrW(dlg, DWLP_USER, lp);
        VideoMeta& m = *d->m;
        d->kind = d->o->kind;
        d->vfmt = d->o->vfmt;
        d->afmt = d->o->afmt;
        d->dir = d->o->dir;

        Ctl(dlg, L"STATIC", m.title.c_str(), SS_LEFT | SS_NOPREFIX, 200, 22, OW - 224, 48, IDC_O_TITLE, g_fontTitle);
        std::wstring meta;
        if (m.isGroup) {
            meta = T(m.groupType == GROUP_CHANNEL ? S_OPT_CHANNEL_WORD : S_OPT_PLAYLIST_WORD);
            meta += Fmt(T(S_OPT_NVIDEOS), (int)m.entries.size());
            if (m.groupType == GROUP_PLAYLIST && !m.channel.empty()) meta += L"  ·  " + m.channel;
        } else {
            meta = m.channel;
            if (m.duration > 0) meta += (meta.empty() ? L"" : L"  ·  ") + FormatDuration(m.duration);
            if (m.uploadDate.size() == 8)
                meta += L"  ·  " + m.uploadDate.substr(0, 4) + L"." + m.uploadDate.substr(4, 2) + L"." + m.uploadDate.substr(6, 2);
        }
        d->meta = Ctl(dlg, L"STATIC", meta.c_str(), SS_LEFT | SS_NOPREFIX, 200, 74, OW - 224, 40, IDC_O_META);

        int y = 146;
        auto label = [&](int sid) { Ctl(dlg, L"STATIC", T(sid), SS_LEFT | SS_NOPREFIX, 24, y + 5, 92, 20, -1); };
        label(S_LBL_KIND);
        Ctl(dlg, L"BUTTON", T(S_KIND_VIDEO), BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 120, y + 2, 92, 24, IDC_O_VIDEO);
        Ctl(dlg, L"BUTTON", T(S_KIND_AUDIO), BS_AUTORADIOBUTTON, 220, y + 2, 92, 24, IDC_O_AUDIO);
        CheckRadioButton(dlg, IDC_O_VIDEO, IDC_O_AUDIO, d->kind == KIND_VIDEO ? IDC_O_VIDEO : IDC_O_AUDIO);
        y += 40;
        label(S_LBL_QUALITY);
        d->cbQ = Ctl(dlg, WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_GROUP, 120, y, 320, 320, IDC_O_QUALITY);
        y += 40;
        label(S_LBL_FORMAT);
        d->cbF = Ctl(dlg, WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 120, y, 200, 240, IDC_O_FORMAT);
        y += 40;
        label(S_LBL_SUBS);
        d->cbS = Ctl(dlg, WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 120, y, 320, 360, IDC_O_SUB);
        y += 40;
        label(S_LBL_AUDIO);
        d->cbA = Ctl(dlg, WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 120, y, 320, 360, IDC_O_ALANG);
        y += 40;
        label(S_LBL_SAVETO);
        d->edDir = Ctl(dlg, L"EDIT", d->dir.c_str(), ES_AUTOHSCROLL | ES_READONLY | WS_TABSTOP, 120, y, OW - 120 - 24 - 100, 26,
                       IDC_O_DIR, nullptr, WS_EX_CLIENTEDGE);
        MakeButton(dlg, IDC_O_BROWSE, T(S_OPT_CHANGE), 0, BTN_SECONDARY, false, OW - 24 - 92, y - 1, 92, 28);
        y += 44;

        if (m.isGroup) {
            d->chkAll = Ctl(dlg, L"BUTTON", T(S_OPT_SELECT_ALL), BS_AUTOCHECKBOX | WS_TABSTOP | WS_GROUP, 24, y, 240, 24, IDC_O_ALL);
            Button_SetCheck(d->chkAll, BST_CHECKED);
            y += 30;
            d->lv = Ctl(dlg, WC_LISTVIEWW, L"", LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | WS_TABSTOP, 24, y, OW - 48, 210,
                        IDC_O_LIST, nullptr, WS_EX_CLIENTEDGE);
            ListView_SetExtendedListViewStyle(d->lv, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
            ThemeControl(d->lv, L"listview");
            LVCOLUMNW col{};
            col.mask = LVCF_TEXT | LVCF_WIDTH;
            col.pszText = (LPWSTR)T(S_OPT_COL_TITLE);
            col.cx = S(OW - 48 - 110);
            ListView_InsertColumn(d->lv, 0, &col);
            col.pszText = (LPWSTR)T(S_OPT_COL_LENGTH);
            col.cx = S(86);
            ListView_InsertColumn(d->lv, 1, &col);
            for (size_t i = 0; i < m.entries.size(); ++i) {
                LVITEMW it{};
                it.mask = LVIF_TEXT;
                it.iItem = (int)i;
                it.pszText = (LPWSTR)m.entries[i].title.c_str();
                ListView_InsertItem(d->lv, &it);
                std::wstring dur = FormatDuration(m.entries[i].duration);
                ListView_SetItemText(d->lv, (int)i, 1, (LPWSTR)dur.c_str());
                ListView_SetCheckState(d->lv, (int)i, TRUE);
            }
            y += 222;
        }
        d->footerY = y;
        d->ok = MakeButton(dlg, IDOK, T(S_OPT_DOWNLOAD), ico::Download, BTN_PRIMARY, true, OW - 24 - 100 - 10 - 136, y + 15, 136, 34);
        MakeButton(dlg, IDCANCEL, T(S_BTN_CANCEL), 0, BTN_SECONDARY, true, OW - 24 - 100, y + 15, 100, 34);
        DlgPlace(dlg, OW, y + 64);

        FillQuality(d);
        FillFormat(d);
        FillSubs(d);
        FillAudioLangs(d);
        UpdateOk(d);
        if (!m.vid.empty()) ThumbRequest(m.vid, dlg, 1, S(160), S(90));
        SetFocus(d->ok);
        return FALSE;
    }
    case WM_APP_THUMB:
        if (d) {
            if (d->thumb) DeleteObject(d->thumb);
            d->thumb = (HBITMAP)lp;
            RECT r{S(24), S(24), S(184), S(114)};
            InvalidateRect(dlg, &r, FALSE);
        } else if (lp) {
            DeleteObject((HBITMAP)lp);
        }
        return TRUE;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(dlg, &ps);
        RECT rc;
        GetClientRect(dlg, &rc);
        RECT th{S(24), S(24), S(184), S(114)};
        if (d && d->thumb) {
            HDC td = CreateCompatibleDC(hdc);
            HGDIOBJ o = SelectObject(td, d->thumb);
            BitBlt(hdc, th.left, th.top, th.right - th.left, th.bottom - th.top, td, 0, 0, SRCCOPY);
            SelectObject(td, o);
            DeleteDC(td);
        } else {
            FillRound(hdc, th, S(6), clr.track);
            bool audio = d && d->kind == KIND_AUDIO;
            DrawTextIn(hdc, std::wstring(1, audio ? ico::Music : ico::Video), th, clr.faint, g_fontIcon,
                       DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        RECT sep{S(24), S(130), rc.right - S(24), S(131)};
        FillSolid(hdc, sep, clr.line);
        if (d) {
            RECT foot{0, S(d->footerY), rc.right, rc.bottom};
            FillSolid(hdc, foot, clr.panel);
            RECT fl{0, S(d->footerY), rc.right, S(d->footerY) + 1};
            FillSolid(hdc, fl, clr.line);
        }
        EndPaint(dlg, &ps);
        return TRUE;
    }
    case WM_CTLCOLORDLG:
        return (INT_PTR)BrushOf(clr.bg);
    case WM_CTLCOLORSTATIC:
        return DlgColor((HDC)wp, d && (HWND)lp == d->meta ? clr.sub : clr.text, clr.bg);
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:  // drop-down part of the combo boxes
        return DlgColor((HDC)wp);
    case WM_DRAWITEM:
        return DrawFancyButton((const DRAWITEMSTRUCT*)lp);
    case WM_NOTIFY: {
        auto* nm = (NMHDR*)lp;
        if (d && nm->idFrom == IDC_O_LIST && nm->code == LVN_ITEMCHANGED) {
            auto* lv = (NMLISTVIEW*)lp;
            if ((lv->uChanged & LVIF_STATE) && ((lv->uNewState ^ lv->uOldState) & LVIS_STATEIMAGEMASK)) UpdateOk(d);
        }
        return FALSE;
    }
    case WM_COMMAND: {
        if (!d) return FALSE;
        int id = LOWORD(wp), code = HIWORD(wp);
        switch (id) {
        case IDC_O_VIDEO:
        case IDC_O_AUDIO:
            if (code == BN_CLICKED) {
                SetKind(d, id == IDC_O_VIDEO ? KIND_VIDEO : KIND_AUDIO);
                RECT th{S(24), S(24), S(184), S(114)};
                InvalidateRect(dlg, &th, FALSE);
            }
            return TRUE;
        case IDC_O_FORMAT:
            if (code == CBN_SELCHANGE) {
                int i = ComboGet(d->cbF);
                if (i >= 0 && i < (int)d->fmtVals.size()) {
                    if (d->kind == KIND_VIDEO) d->vfmt = d->fmtVals[i];
                    else {
                        d->afmt = d->fmtVals[i];
                        FillQuality(d);
                    }
                }
            }
            return TRUE;
        case IDC_O_BROWSE: {
            std::wstring p = d->dir;
            if (PickFolder(dlg, p, T(S_PICK_FOLDER))) {
                d->dir = p;
                d->customDir = true;
                SetWindowTextW(d->edDir, p.c_str());
            }
            return TRUE;
        }
        case IDC_O_ALL:
            if (code == BN_CLICKED && d->lv) {
                BOOL on = Button_GetCheck(d->chkAll) == BST_CHECKED;
                int n = ListView_GetItemCount(d->lv);
                for (int i = 0; i < n; ++i) ListView_SetCheckState(d->lv, i, on);
                UpdateOk(d);
            }
            return TRUE;
        case IDOK: {
            if (d->lv && CheckedCount(d) == 0) return TRUE;
            DlOptions& o = *d->o;
            o.kind = d->kind;
            int qi = ComboGet(d->cbQ);
            if (d->kind == KIND_VIDEO) {
                if (qi >= 0 && qi < (int)d->qVals.size()) o.height = d->qVals[qi];
                o.vfmt = d->vfmt;
            } else {
                if (d->afmt == L"mp3" && qi >= 0 && qi < (int)d->aqVals.size()) o.aq = d->aqVals[qi];
                o.afmt = d->afmt;
            }
            int si = ComboGet(d->cbS);
            if (si >= 0 && si < (int)d->subVals.size()) {
                o.sub = d->subVals[si].code;
                o.subAuto = d->m->isGroup ? o.subAuto : d->subVals[si].autoGen;
            }
            int ai = ComboGet(d->cbA);
            o.alang = (ai >= 0 && ai < (int)d->alVals.size()) ? d->alVals[ai] : L"";
            o.dir = d->dir;
            if (d->lv)
                for (size_t i = 0; i < d->m->entries.size(); ++i) d->m->entries[i].checked = ListView_GetCheckState(d->lv, (int)i) != 0;
            EndDialog(dlg, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(dlg, IDCANCEL);
            return TRUE;
        }
        return FALSE;
    }
    case WM_DESTROY:
        if (d && d->thumb) {
            DeleteObject(d->thumb);
            d->thumb = nullptr;
        }
        return FALSE;
    }
    return FALSE;
}

}  // namespace

bool ShowOptionsDialog(HWND owner, VideoMeta& meta, DlOptions& opt) {
    OptDlg d;
    d.m = &meta;
    d.o = &opt;
    return RunDialog(owner, T(S_OPT_TITLE), OptProc, (LPARAM)&d) == IDOK;
}

// ====================================================================== settings dialog

namespace {

enum {
    IDC_S_NAV = 2001,
    IDC_S_TITLE,
    IDC_S_NAME,
    IDC_S_GROUP,
    IDC_S_META,
    IDC_S_AUTOSUB,
    IDC_S_QUICK,
    IDC_S_CONC,
    IDC_S_FPS,
    IDC_S_CODEC,
    IDC_S_RATE,
    IDC_S_THEME,
    IDC_S_LANG,
    IDC_S_USEPROXY,
    IDC_S_PROXY,
    IDC_S_RETRY,
    IDC_S_NOTIFY,
    IDC_S_NOTIFYERR,
    IDC_S_SOUND,
    IDC_S_BROWSER,
    IDC_S_COOKIE,
    IDC_S_COOKIEBROWSE,
    IDC_S_YTDLP,
    IDC_S_YTDLPBROWSE,
    IDC_S_FFMPEG,
    IDC_S_FFMPEGBROWSE,
    IDC_S_UPDATE,
    IDC_S_AUTOUPDATE,
};

constexpr int SW = 700, SH = 486, SFOOT = 432, SIDE = 200, PX = 228;
constexpr int kPageCount = 7;
const int kPageName[kPageCount] = {S_PAGE_GENERAL, S_PAGE_DOWNLOAD, S_PAGE_APPEARANCE, S_PAGE_CONNECTION,
                                   S_PAGE_NOTIFY,  S_PAGE_LOGIN,    S_PAGE_TOOLS};
const wchar_t kPageIcon[kPageCount] = {0xE713, 0xE896, 0xE790, 0xE774, 0xEA8F, 0xE77B, 0xE90F};

struct SetDlg {
    Settings s;
    const Tools* tools = nullptr;
    std::wstring ver;
    HWND nav = nullptr, title = nullptr;
    std::vector<HWND> page[kPageCount];
    std::set<HWND> notes;
    int cur = 0;
    HWND name, group, meta, autosub, quick, conc, fps, codec, rate, theme, lang, useProxy, proxy, retry, notify,
        notifyErr, sound, browser, cookie, ytdlp, ffmpeg, autoUpdate;
};

void ShowPage(SetDlg* d, HWND dlg, int p) {
    d->cur = p;
    for (int i = 0; i < kPageCount; ++i)
        for (HWND h : d->page[i]) ShowWindow(h, i == p ? SW_SHOW : SW_HIDE);
    SetWindowTextW(d->title, T(kPageName[p]));
    SendMessageW(d->nav, LB_SETCURSEL, p, 0);
    RECT r{S(SIDE), 0, S(SW), S(SFOOT)};
    InvalidateRect(dlg, &r, TRUE);
}

INT_PTR CALLBACK SetProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp) {
    auto* d = (SetDlg*)GetWindowLongPtrW(dlg, DWLP_USER);
    switch (msg) {
    case WM_INITDIALOG: {
        d = (SetDlg*)lp;
        SetWindowLongPtrW(dlg, DWLP_USER, lp);
        Settings& s = d->s;
        d->nav = Ctl(dlg, L"LISTBOX", L"", LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_TABSTOP, 12, 16, SIDE - 24,
                     SFOOT - 32, IDC_S_NAV);
        for (int i = 0; i < kPageCount; ++i) SendMessageW(d->nav, LB_ADDSTRING, 0, (LPARAM)i);
        d->title = Ctl(dlg, L"STATIC", L"", SS_LEFT | SS_NOPREFIX, PX, 18, SW - PX - 24, 32, IDC_S_TITLE, g_fontHeading);

        int P = 0;
        int CW = SW - PX - 24;  // content width
        auto add = [&](HWND h) {
            d->page[P].push_back(h);
            return h;
        };
        auto label = [&](int sid, int x, int y, int w = 130) {
            return add(Ctl(dlg, L"STATIC", T(sid), SS_LEFT | SS_NOPREFIX, x, y, w, 20, -1));
        };
        auto note = [&](int sid, int y, int h) {
            HWND n = add(Ctl(dlg, L"STATIC", T(sid), SS_LEFT | SS_NOPREFIX, PX, y, CW, h, -1, g_fontSmall));
            d->notes.insert(n);
            return n;
        };
        auto noteText = [&](const std::wstring& text, int y, int h) {
            HWND n = add(Ctl(dlg, L"STATIC", text.c_str(), SS_LEFT | SS_NOPREFIX, PX, y, CW, h, -1, g_fontSmall));
            d->notes.insert(n);
            return n;
        };
        auto check = [&](int sid, int id, int y, bool on) {
            HWND c = add(Ctl(dlg, L"BUTTON", T(sid), BS_AUTOCHECKBOX | BS_MULTILINE | WS_TABSTOP, PX, y, CW, 24, id));
            Button_SetCheck(c, on ? BST_CHECKED : BST_UNCHECKED);
            return c;
        };
        auto combo = [&](int id, int x, int y, int w, const std::vector<std::wstring>& items, int sel) {
            HWND c = add(Ctl(dlg, WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, x, y, w, 320, id));
            ComboSet(c, items, sel);
            return c;
        };
        auto edit = [&](int id, int x, int y, int w, const std::wstring& text, int cueSid) {
            HWND e = add(Ctl(dlg, L"EDIT", text.c_str(), ES_AUTOHSCROLL | WS_TABSTOP, x, y, w, 26, id, nullptr, WS_EX_CLIENTEDGE));
            if (cueSid >= 0) SendMessageW(e, EM_SETCUEBANNER, TRUE, (LPARAM)T(cueSid));
            return e;
        };

        // --- General
        P = 0;
        label(S_SET_FILENAME, PX, 74, 110);
        d->name = combo(IDC_S_NAME, PX + 120, 70, CW - 120, Labels(kNameTemplates), s.nameTpl);
        d->group = check(S_SET_GROUPFOLDERS, IDC_S_GROUP, 116, s.groupFolders);
        d->meta = check(S_SET_EMBED, IDC_S_META, 146, s.embedMeta);
        d->autosub = check(S_SET_AUTOSUBS, IDC_S_AUTOSUB, 176, s.autoSubs);
        d->quick = check(S_SET_QUICK, IDC_S_QUICK, 206, s.quick);
        note(S_SET_NOTE_SKIP, 248, 40);

        // --- Downloads
        P = 1;
        label(S_SET_CONCURRENT, PX, 74, 150);
        {
            // Say what each step costs, the way 4K Video Downloader does: the
            // number alone gives no hint that the top of the range invites a
            // temporary block from YouTube.
            std::vector<std::wstring> items;
            for (int i = 1; i <= kMaxConcurrent; ++i) {
                int tone = i <= 3 ? S_CONC_SAFE : i <= 5 ? S_CONC_STABLE : i <= 7 ? S_CONC_FAST : S_CONC_RISKY;
                items.push_back(Fmt(T(S_SET_CONCURRENT_N), i) + L"   ·   " + T(tone));
            }
            d->conc = combo(IDC_S_CONC, PX + 160, 70, 150, items, std::max(0, std::min(kMaxConcurrent - 1, s.concurrency - 1)));
        }
        note(S_SET_CONCURRENT_NOTE, 104, 40);
        label(S_SET_FPS, PX, 158, 150);
        d->fps = combo(IDC_S_FPS, PX + 160, 154, CW - 160, Labels(kFps), s.fps);
        label(S_SET_CODEC, PX, 196, 150);
        d->codec = combo(IDC_S_CODEC, PX + 160, 192, CW - 160, Labels(kCodecs), s.codec);
        label(S_SET_RATE, PX, 234, 150);
        d->rate = combo(IDC_S_RATE, PX + 160, 230, CW - 160, Labels(kRateLimits), s.rate);
        note(S_SET_CODEC_NOTE, 272, 56);

        // --- Appearance
        P = 2;
        label(S_SET_THEME, PX, 78, 110);
        d->theme = combo(IDC_S_THEME, PX + 120, 74, CW - 120, Labels(kThemes), s.theme);
        label(S_SET_LANGUAGE, PX, 126, 110);
        d->lang = combo(IDC_S_LANG, PX + 120, 122, CW - 120, LanguageLabels(), s.lang);
        note(S_SET_APPEARANCE_NOTE, 170, 48);

        // --- Connection
        P = 3;
        d->useProxy = check(S_SET_USEPROXY, IDC_S_USEPROXY, 72, s.useProxy);
        d->proxy = edit(IDC_S_PROXY, PX, 102, CW, s.proxy, S_SET_PROXY_CUE);
        label(S_SET_RETRY, PX, 154, 150);
        {
            std::vector<std::wstring> items;
            for (int i = 0; i < 4; ++i) items.push_back(Fmt(T(S_SET_RETRY_N), kRetries[i]));
            d->retry = combo(IDC_S_RETRY, PX + 160, 150, 150, items, s.retries);
        }
        note(S_SET_CONN_NOTE, 190, 40);

        // --- Notifications
        P = 4;
        label(S_SET_NOTIFY_WHEN, PX, 78, 150);
        d->notify = combo(IDC_S_NOTIFY, PX + 160, 74, CW - 160, Labels(kNotifyWhen), s.notify);
        d->notifyErr = check(S_SET_NOTIFY_ERR, IDC_S_NOTIFYERR, 118, s.notifyErrors);
        d->sound = check(S_SET_SOUND, IDC_S_SOUND, 148, s.sound);
        note(S_SET_NOTIFY_NOTE, 190, 40);

        // --- Sign-in
        P = 5;
        note(S_SET_LOGIN_NOTE, 66, 64);
        label(S_SET_BROWSER, PX, 146, 110);
        d->browser = combo(IDC_S_BROWSER, PX + 120, 142, CW - 120, Labels(kBrowsers), s.browser);
        label(S_SET_COOKIE, PX, 184, 110);
        d->cookie = edit(IDC_S_COOKIE, PX + 120, 180, CW - 120 - 96, s.cookieFile, S_SET_COOKIE_CUE);
        add(MakeButton(dlg, IDC_S_COOKIEBROWSE, T(S_SET_BROWSE), 0, BTN_SECONDARY, false, PX + CW - 88, 179, 88, 28));
        note(S_SET_COOKIE_NOTE, 222, 72);

        // --- Tools
        P = 6;
        label(S_SET_YTDLP_PATH, PX, 66, 150);
        d->ytdlp = edit(IDC_S_YTDLP, PX, 88, CW - 96, s.ytdlpPath, S_SET_AUTOFIND_CUE);
        add(MakeButton(dlg, IDC_S_YTDLPBROWSE, T(S_SET_BROWSE), 0, BTN_SECONDARY, false, PX + CW - 88, 87, 88, 28));
        noteText(Fmt(T(S_SET_INUSE), d->tools->ytdlp.empty() ? T(S_SET_NOTFOUND)
                                                             : (d->tools->ytdlp + (d->ver.empty() ? L"" : L"  (" + d->ver + L")")).c_str()),
                 118, 36);
        label(S_SET_FFMPEG_PATH, PX, 160, 150);
        d->ffmpeg = edit(IDC_S_FFMPEG, PX, 182, CW - 96, s.ffmpegPath, S_SET_AUTOFIND_CUE);
        add(MakeButton(dlg, IDC_S_FFMPEGBROWSE, T(S_SET_BROWSE), 0, BTN_SECONDARY, false, PX + CW - 88, 181, 88, 28));
        noteText(Fmt(T(S_SET_INUSE), d->tools->ffmpegExe.empty() ? T(S_SET_NOTFOUND_FF) : d->tools->ffmpegExe.c_str()), 212, 36);
        d->autoUpdate = check(S_SET_AUTOUPDATE, IDC_S_AUTOUPDATE, 254, s.autoUpdate);
        add(MakeButton(dlg, IDC_S_UPDATE, T(S_SET_UPDATE_NOW), 0xE72C, BTN_SECONDARY, false, PX, 288, 180, 32));
        note(S_SET_TOOLS_NOTE, 330, 56);

        MakeButton(dlg, IDOK, T(S_BTN_OK), 0, BTN_PRIMARY, true, SW - 24 - 100 - 10 - 100, SFOOT + 12, 100, 34);
        MakeButton(dlg, IDCANCEL, T(S_BTN_CANCEL), 0, BTN_SECONDARY, true, SW - 24 - 100, SFOOT + 12, 100, 34);
        DlgPlace(dlg, SW, SH);
        ShowPage(d, dlg, 0);
        SetFocus(d->nav);
        return FALSE;
    }
    case WM_MEASUREITEM: {
        auto* mi = (MEASUREITEMSTRUCT*)lp;
        if (mi->CtlID == IDC_S_NAV) {
            mi->itemHeight = S(38);
            return TRUE;
        }
        return FALSE;
    }
    case WM_DRAWITEM: {
        auto* di = (const DRAWITEMSTRUCT*)lp;
        if (di->CtlID == IDC_S_NAV) {
            if ((int)di->itemID < 0) return TRUE;
            RECT r = di->rcItem;
            int w = r.right - r.left, h = r.bottom - r.top;
            HDC mem = CreateCompatibleDC(di->hDC);
            HBITMAP bmp = CreateCompatibleBitmap(di->hDC, w, h);
            HGDIOBJ ob = SelectObject(mem, bmp);
            RECT lr{0, 0, w, h};
            FillSolid(mem, lr, clr.panel);
            bool sel = di->itemState & ODS_SELECTED;
            if (sel) {
                RECT b{0, S(3), w, h - S(3)};
                FillRound(mem, b, S(6), clr.bg, clr.line);
                RECT bar{0, S(11), S(3), h - S(11)};
                FillRound(mem, bar, S(1), clr.accent);
            }
            int i = (int)di->itemData;
            RECT ir{S(14), 0, S(40), h};
            DrawTextIn(mem, std::wstring(1, kPageIcon[i]), ir, sel ? clr.accent : clr.sub, g_fontIconSmall,
                       DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            RECT tr{S(42), 0, w - S(6), h};
            DrawTextIn(mem, T(kPageName[i]), tr, sel ? clr.text : clr.sub, sel ? g_fontBold : g_font,
                       DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            BitBlt(di->hDC, r.left, r.top, w, h, mem, 0, 0, SRCCOPY);
            SelectObject(mem, ob);
            DeleteObject(bmp);
            DeleteDC(mem);
            return TRUE;
        }
        return DrawFancyButton(di);
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(dlg, &ps);
        RECT rc;
        GetClientRect(dlg, &rc);
        RECT side{0, 0, S(SIDE), S(SFOOT)};
        FillSolid(hdc, side, clr.panel);
        RECT sl{S(SIDE), 0, S(SIDE) + 1, S(SFOOT)};
        FillSolid(hdc, sl, clr.line);
        RECT foot{0, S(SFOOT), rc.right, rc.bottom};
        FillSolid(hdc, foot, clr.panel);
        RECT fl{0, S(SFOOT), rc.right, S(SFOOT) + 1};
        FillSolid(hdc, fl, clr.line);
        EndPaint(dlg, &ps);
        return TRUE;
    }
    case WM_CTLCOLORDLG:
        return (INT_PTR)BrushOf(clr.bg);
    case WM_CTLCOLORLISTBOX:
        if (d && (HWND)lp == d->nav) return DlgColor((HDC)wp, clr.text, clr.panel);
        return DlgColor((HDC)wp);  // drop-down part of the combo boxes
    case WM_CTLCOLORSTATIC:
        return DlgColor((HDC)wp, d && d->notes.count((HWND)lp) ? clr.sub : clr.text, clr.bg);
    case WM_CTLCOLOREDIT:
        return DlgColor((HDC)wp);
    case WM_COMMAND: {
        if (!d) return FALSE;
        int id = LOWORD(wp), code = HIWORD(wp);
        switch (id) {
        case IDC_S_NAV:
            if (code == LBN_SELCHANGE) {
                int i = (int)SendMessageW(d->nav, LB_GETCURSEL, 0, 0);
                if (i >= 0) ShowPage(d, dlg, i);
            }
            return TRUE;
        case IDC_S_COOKIEBROWSE: {
            std::wstring p = GetText(d->cookie);
            if (PickFile(dlg, p, T(S_PICK_COOKIE), T(S_PICK_COOKIE_FILTER), L"*.txt")) SetWindowTextW(d->cookie, p.c_str());
            return TRUE;
        }
        case IDC_S_YTDLPBROWSE: {
            std::wstring p = GetText(d->ytdlp);
            if (PickFile(dlg, p, T(S_PICK_YTDLP), L"yt-dlp.exe", L"*.exe")) SetWindowTextW(d->ytdlp, p.c_str());
            return TRUE;
        }
        case IDC_S_FFMPEGBROWSE: {
            std::wstring p = GetText(d->ffmpeg);
            if (PickFile(dlg, p, T(S_PICK_FFMPEG), L"ffmpeg.exe", L"ffmpeg.exe")) SetWindowTextW(d->ffmpeg, p.c_str());
            return TRUE;
        }
        case IDC_S_UPDATE:
            PostMessageW(GetWindow(dlg, GW_OWNER), WM_COMMAND, IDM_UPDATE_YTDLP, 0);
            return TRUE;
        case IDOK: {
            Settings& s = d->s;
            s.nameTpl = std::max(0, ComboGet(d->name));
            s.groupFolders = Button_GetCheck(d->group) == BST_CHECKED;
            s.embedMeta = Button_GetCheck(d->meta) == BST_CHECKED;
            s.autoSubs = Button_GetCheck(d->autosub) == BST_CHECKED;
            s.quick = Button_GetCheck(d->quick) == BST_CHECKED;
            s.concurrency = std::max(0, ComboGet(d->conc)) + 1;
            s.fps = std::max(0, ComboGet(d->fps));
            s.codec = std::max(0, ComboGet(d->codec));
            s.rate = std::max(0, ComboGet(d->rate));
            s.theme = std::max(0, ComboGet(d->theme));
            s.lang = std::max(0, ComboGet(d->lang));
            s.useProxy = Button_GetCheck(d->useProxy) == BST_CHECKED;
            s.proxy = Trim(GetText(d->proxy));
            s.retries = std::max(0, ComboGet(d->retry));
            s.notify = std::max(0, ComboGet(d->notify));
            s.notifyErrors = Button_GetCheck(d->notifyErr) == BST_CHECKED;
            s.sound = Button_GetCheck(d->sound) == BST_CHECKED;
            s.browser = std::max(0, ComboGet(d->browser));
            s.cookieFile = Trim(GetText(d->cookie));
            s.ytdlpPath = Trim(GetText(d->ytdlp));
            s.ffmpegPath = Trim(GetText(d->ffmpeg));
            s.autoUpdate = Button_GetCheck(d->autoUpdate) == BST_CHECKED;
            auto fail = [&](int page, int sid, HWND focus) {
                ShowPage(d, dlg, page);
                MessageBoxW(dlg, T(sid), APP_NAME, MB_ICONWARNING);
                SetFocus(focus);
            };
            if (s.useProxy && s.proxy.empty()) {
                fail(3, S_SET_ERR_PROXY, d->proxy);
                return TRUE;
            }
            if (!s.cookieFile.empty() && !FileExists(s.cookieFile)) {
                fail(5, S_SET_ERR_COOKIE, d->cookie);
                return TRUE;
            }
            if (!s.ytdlpPath.empty() && !FileExists(s.ytdlpPath)) {
                fail(6, S_SET_ERR_YTDLP, d->ytdlp);
                return TRUE;
            }
            if (!s.ffmpegPath.empty() && !FileExists(s.ffmpegPath) && !DirExists(s.ffmpegPath)) {
                fail(6, S_SET_ERR_FFMPEG, d->ffmpeg);
                return TRUE;
            }
            EndDialog(dlg, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(dlg, IDCANCEL);
            return TRUE;
        }
        return FALSE;
    }
    }
    return FALSE;
}

}  // namespace

bool ShowSettingsDialog(HWND owner, const Tools& tools, const std::wstring& ytdlpVersion, bool& uiChanged) {
    SetDlg d;
    d.s = g_set;
    d.tools = &tools;
    d.ver = ytdlpVersion;
    if (RunDialog(owner, T(S_SET_TITLE), SetProc, (LPARAM)&d) != IDOK) return false;
    uiChanged = d.s.lang != g_set.lang || d.s.theme != g_set.theme;
    g_set = d.s;
    return true;
}
