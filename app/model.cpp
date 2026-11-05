#include "model.h"
#include "json.h"
#include <cwctype>

Settings g_set;

std::wstring Choice::label() const { return sid >= 0 ? T(sid) : fixed; }

const std::vector<Choice> kVideoQuality = {
    {S_Q_BEST_VIDEO, nullptr, L"0"}, {-1, L"8K (4320p)", L"4320"}, {-1, L"4K (2160p)", L"2160"},
    {-1, L"2K (1440p)", L"1440"},    {-1, L"1080p", L"1080"},      {-1, L"720p", L"720"},
    {-1, L"480p", L"480"},           {-1, L"360p", L"360"},        {-1, L"240p", L"240"},
    {-1, L"144p", L"144"},
};
const std::vector<Choice> kAudioQuality = {
    {S_Q_BEST_AUDIO, nullptr, L"0"}, {-1, L"320 kbps", L"320K"}, {-1, L"256 kbps", L"256K"},
    {-1, L"192 kbps", L"192K"},      {-1, L"128 kbps", L"128K"},
};
const std::vector<Choice> kVideoFormat = {{-1, L"MP4", L"mp4"}, {-1, L"MKV", L"mkv"}};
const std::vector<Choice> kAudioFormat = {{-1, L"MP3", L"mp3"}, {-1, L"M4A", L"m4a"}};
const std::vector<Choice> kNameTemplates = {
    {S_NT_TITLE, nullptr, L"%(title)s"},
    {S_NT_CHANNEL_TITLE, nullptr, L"%(channel,uploader|?)s - %(title)s"},
    {S_NT_DATE_TITLE, nullptr, L"%(upload_date>%Y-%m-%d)s %(title)s"},
    {S_NT_TITLE_ID, nullptr, L"%(title)s [%(id)s]"},
};
const std::vector<Choice> kCodecs = {{S_CODEC_H264, nullptr, L"avc"}, {-1, L"VP9", L"vp9"}, {S_CODEC_AV1, nullptr, L"av01"}};
const std::vector<Choice> kFps = {{S_FPS_BEST, nullptr, L""}, {S_FPS_30, nullptr, L"30"}};
const std::vector<Choice> kRateLimits = {{S_RATE_NONE, nullptr, L""}, {-1, L"1 MB/s", L"1M"}, {-1, L"2 MB/s", L"2M"},
                                         {-1, L"5 MB/s", L"5M"},      {-1, L"10 MB/s", L"10M"}};
const std::vector<Choice> kBrowsers = {
    {S_BROWSER_NONE, nullptr, L""},  {-1, L"Firefox", L"firefox"}, {-1, L"Chrome", L"chrome"},
    {-1, L"Microsoft Edge", L"edge"}, {-1, L"Brave", L"brave"},     {-1, L"Whale", L"whale"},
    {-1, L"Opera", L"opera"},        {-1, L"Vivaldi", L"vivaldi"},
};
const std::vector<Choice> kNotifyWhen = {{S_NOTIFY_OFF, nullptr, L""}, {S_NOTIFY_ALL, nullptr, L""}, {S_NOTIFY_EACH, nullptr, L""}};
const std::vector<Choice> kThemes = {{S_THEME_SYSTEM, nullptr, L""}, {S_THEME_LIGHT, nullptr, L""}, {S_THEME_DARK, nullptr, L""}};
const int kRetries[4] = {3, 5, 10, 20};

std::vector<std::wstring> Labels(const std::vector<Choice>& c) {
    std::vector<std::wstring> r;
    for (auto& x : c) r.push_back(x.label());
    return r;
}

std::vector<std::wstring> LanguageLabels() {
    std::vector<std::wstring> r;
    for (int i = 0; i < LANG_COUNT; ++i) r.push_back(kLangs[i].native);
    return r;
}

// ---- subtitle / audio pickers -------------------------------------------------

std::vector<std::wstring> SubtitleLabels() {
    std::vector<std::wstring> r;
    r.push_back(T(S_SUB_NONE));
    for (int i = 0; i < kSubtitleCodeCount; ++i) r.push_back(NativeLangName(kSubtitleCodes[i]));
    r.push_back(T(S_SUB_ALL));
    return r;
}

std::wstring SubtitleCodeAt(int index) {
    if (index <= 0) return L"";
    if (index > kSubtitleCodeCount) return L"all";
    return kSubtitleCodes[index - 1];
}

int SubtitleIndexOf(const std::wstring& code) {
    if (code.empty()) return 0;
    if (code == L"all") return kSubtitleCodeCount + 1;
    for (int i = 0; i < kSubtitleCodeCount; ++i)
        if (code == kSubtitleCodes[i]) return i + 1;
    return 0;
}

std::vector<std::wstring> AudioLangLabels() {
    std::vector<std::wstring> r;
    r.push_back(T(S_AL_DEFAULT));
    for (int i = 0; i < kSubtitleCodeCount; ++i) r.push_back(NativeLangName(kSubtitleCodes[i]));
    return r;
}

std::wstring AudioLangCodeAt(int index) { return index <= 0 ? L"" : kSubtitleCodes[index - 1]; }

int AudioLangIndexOf(const std::wstring& code) {
    if (code.empty()) return 0;
    for (int i = 0; i < kSubtitleCodeCount; ++i)
        if (code == kSubtitleCodes[i]) return i + 1;
    return 0;
}

// ============================================================ settings

static std::wstring IniPath() { return PathJoin(AppDataDir(), L"settings.ini"); }

static std::wstring RS(const wchar_t* key, const std::wstring& def) {
    std::vector<wchar_t> buf(4096);
    GetPrivateProfileStringW(L"settings", key, def.c_str(), buf.data(), (DWORD)buf.size(), IniPath().c_str());
    return buf.data();
}
static int RI(const wchar_t* key, int def, int lo, int hi) {
    std::wstring s = RS(key, L"");
    if (s.empty()) return def;
    int v = _wtoi(s.c_str());
    return std::max(lo, std::min(hi, v));
}
static void WS(const wchar_t* key, const std::wstring& v) {
    WritePrivateProfileStringW(L"settings", key, v.c_str(), IniPath().c_str());
}
static void WI(const wchar_t* key, int v) { WS(key, std::to_wstring(v)); }

void LoadSettings() {
    Settings d;
    d.dirVideo = KnownFolder(FOLDERID_Videos);
    d.dirAudio = KnownFolder(FOLDERID_Music);
    if (d.dirVideo.empty()) d.dirVideo = KnownFolder(FOLDERID_Downloads);
    if (d.dirAudio.empty()) d.dirAudio = d.dirVideo;
    g_set = d;
    if (!FileExists(IniPath())) {
        // First run: take the language and any tool paths recorded by the installer.
        std::wstring def = PathJoin(ExeDir(), L"default.ini");
        auto read = [&](const wchar_t* key) {
            std::vector<wchar_t> buf(1024);
            GetPrivateProfileStringW(L"settings", key, L"", buf.data(), (DWORD)buf.size(), def.c_str());
            return std::wstring(buf.data());
        };
        int lang = LangFromCode(read(L"language").c_str());
        g_set.lang = lang >= 0 ? lang : LANG_KO;  // Korean is the product default
        g_lang = g_set.lang;
        g_set.ytdlpPath = read(L"ytdlpPath");
        g_set.ffmpegPath = read(L"ffmpegPath");
        return;
    }
    Settings& s = g_set;
    int lang = LangFromCode(RS(L"language", L"").c_str());
    s.lang = lang >= 0 ? lang : LangFromSystem();
    g_lang = s.lang;
    s.theme = RI(L"theme", d.theme, 0, 2);
    s.kind = RI(L"kind", d.kind, 0, 1);
    s.vq = RI(L"videoQuality", d.vq, 0, (int)kVideoQuality.size() - 1);
    s.aq = RI(L"audioQuality", d.aq, 0, (int)kAudioQuality.size() - 1);
    s.vfmt = RI(L"videoFormat", d.vfmt, 0, (int)kVideoFormat.size() - 1);
    s.afmt = RI(L"audioFormat", d.afmt, 0, (int)kAudioFormat.size() - 1);
    s.subLang = RS(L"subtitleLanguage", d.subLang);
    s.audioLang = RS(L"audioLanguage", d.audioLang);
    s.dirVideo = RS(L"videoFolder", d.dirVideo);
    s.dirAudio = RS(L"audioFolder", d.dirAudio);
    s.quick = RI(L"quickMode", d.quick, 0, 1) != 0;
    s.nameTpl = RI(L"nameTemplate", d.nameTpl, 0, (int)kNameTemplates.size() - 1);
    s.groupFolders = RI(L"groupFolders", d.groupFolders, 0, 1) != 0;
    s.embedMeta = RI(L"embedMetadata", d.embedMeta, 0, 1) != 0;
    s.autoSubs = RI(L"autoSubtitles", d.autoSubs, 0, 1) != 0;
    s.concurrency = RI(L"simultaneousDownloads", d.concurrency, 1, kMaxConcurrent);
    s.fps = RI(L"fps", d.fps, 0, (int)kFps.size() - 1);
    s.codec = RI(L"codec", d.codec, 0, (int)kCodecs.size() - 1);
    s.rate = RI(L"rateLimit", d.rate, 0, (int)kRateLimits.size() - 1);
    s.useProxy = RI(L"useProxy", d.useProxy, 0, 1) != 0;
    s.proxy = RS(L"proxy", d.proxy);
    s.retries = RI(L"retries", d.retries, 0, 3);
    s.notify = RI(L"notify", d.notify, 0, 2);
    s.notifyErrors = RI(L"notifyErrors", d.notifyErrors, 0, 1) != 0;
    s.sound = RI(L"sound", d.sound, 0, 1) != 0;
    s.browser = RI(L"cookiesBrowser", d.browser, 0, (int)kBrowsers.size() - 1);
    s.cookieFile = RS(L"cookiesFile", d.cookieFile);
    s.ytdlpPath = RS(L"ytdlpPath", L"");
    s.ffmpegPath = RS(L"ffmpegPath", L"");
    s.autoUpdate = RI(L"autoUpdate", d.autoUpdate, 0, 1) != 0;
    s.lastUpdateCheck = _wtoi64(RS(L"lastUpdateCheck", L"0").c_str());
    s.hasPos = RI(L"hasWindowPos", 0, 0, 1) != 0;
    s.x = RI(L"windowX", 0, -100000, 100000);
    s.y = RI(L"windowY", 0, -100000, 100000);
    s.w = RI(L"windowW", 0, 0, 100000);
    s.h = RI(L"windowH", 0, 0, 100000);
    s.maximized = RI(L"windowMaximized", 0, 0, 1) != 0;
    s.tab = RI(L"tab", 0, 0, 4);
    s.sort = RI(L"sort", 0, 0, 4);
    if (s.dirVideo.empty()) s.dirVideo = d.dirVideo;
    if (s.dirAudio.empty()) s.dirAudio = d.dirAudio;
}

void SaveSettings() {
    EnsureDir(AppDataDir());
    std::wstring ini = IniPath();
    if (!FileExists(ini)) WriteFileBytes(ini, std::string("\xFF\xFE", 2));  // UTF-16 so paths keep Unicode
    const Settings& s = g_set;
    WS(L"language", kLangs[(s.lang >= 0 && s.lang < LANG_COUNT) ? s.lang : LANG_KO].code);
    WI(L"theme", s.theme);
    WI(L"kind", s.kind);
    WI(L"videoQuality", s.vq);
    WI(L"audioQuality", s.aq);
    WI(L"videoFormat", s.vfmt);
    WI(L"audioFormat", s.afmt);
    WS(L"subtitleLanguage", s.subLang);
    WS(L"audioLanguage", s.audioLang);
    WS(L"videoFolder", s.dirVideo);
    WS(L"audioFolder", s.dirAudio);
    WI(L"quickMode", s.quick);
    WI(L"nameTemplate", s.nameTpl);
    WI(L"groupFolders", s.groupFolders);
    WI(L"embedMetadata", s.embedMeta);
    WI(L"autoSubtitles", s.autoSubs);
    WI(L"simultaneousDownloads", s.concurrency);
    WI(L"fps", s.fps);
    WI(L"codec", s.codec);
    WI(L"rateLimit", s.rate);
    WI(L"useProxy", s.useProxy);
    WS(L"proxy", s.proxy);
    WI(L"retries", s.retries);
    WI(L"notify", s.notify);
    WI(L"notifyErrors", s.notifyErrors);
    WI(L"sound", s.sound);
    WI(L"cookiesBrowser", s.browser);
    WS(L"cookiesFile", s.cookieFile);
    WS(L"ytdlpPath", s.ytdlpPath);
    WS(L"ffmpegPath", s.ffmpegPath);
    WI(L"autoUpdate", s.autoUpdate);
    WS(L"lastUpdateCheck", std::to_wstring(s.lastUpdateCheck));
    WI(L"hasWindowPos", s.hasPos);
    WI(L"windowX", s.x);
    WI(L"windowY", s.y);
    WI(L"windowW", s.w);
    WI(L"windowH", s.h);
    WI(L"windowMaximized", s.maximized);
    WI(L"tab", s.tab);
    WI(L"sort", s.sort);
}

// ============================================================ tools

std::wstring UserToolsDir() { return PathJoin(LocalDataDir(), L"tools"); }

Tools FindTools() {
    Tools t;
    std::wstring base = ExeDir();
    if (!g_set.ytdlpPath.empty() && FileExists(g_set.ytdlpPath)) {
        t.ytdlp = g_set.ytdlpPath;
    } else {
        // A copy in the user profile wins: when the app lives in Program Files the
        // bundled yt-dlp cannot be updated, so updates are kept per user.
        for (auto& c : {PathJoin(UserToolsDir(), L"yt-dlp.exe"), PathJoin(base, L"tools\\yt-dlp.exe"),
                        PathJoin(base, L"yt-dlp.exe"), FindOnPath(L"yt-dlp.exe")})
            if (FileExists(c)) { t.ytdlp = c; break; }
    }
    t.ownCopy = !t.ytdlp.empty() && DirWritable(PathDir(t.ytdlp));

    std::wstring ff = g_set.ffmpegPath;
    if (!ff.empty() && DirExists(ff)) ff = PathJoin(ff, L"ffmpeg.exe");
    if (!ff.empty() && FileExists(ff)) {
        t.ffmpegExe = ff;
    } else {
        for (auto& c : {PathJoin(base, L"tools\\ffmpeg\\ffmpeg.exe"), PathJoin(base, L"tools\\ffmpeg.exe"),
                        PathJoin(base, L"ffmpeg.exe"), FindOnPath(L"ffmpeg.exe")})
            if (FileExists(c)) { t.ffmpegExe = c; break; }
    }
    if (!t.ffmpegExe.empty()) t.ffmpegDir = PathDir(t.ffmpegExe);

    for (auto& c : {PathJoin(UserToolsDir(), L"qjs.exe"), PathJoin(base, L"tools\\qjs.exe"),
                    t.ytdlp.empty() ? std::wstring() : PathJoin(PathDir(t.ytdlp), L"qjs.exe"), FindOnPath(L"qjs.exe")})
        if (FileExists(c)) { t.qjs = c; break; }
    return t;
}

// ============================================================ links

static std::wstring QueryParam(const std::wstring& query, const wchar_t* key) {
    for (auto& kv : Split(query, L'&')) {
        size_t eq = kv.find(L'=');
        if (kv.substr(0, eq) == key) return eq == std::wstring::npos ? L"" : kv.substr(eq + 1);
    }
    return L"";
}

static bool ValidId(const std::wstring& s) {
    if (s.size() != 11) return false;
    for (wchar_t c : s)
        if (!(iswalnum(c) || c == L'-' || c == L'_')) return false;
    return true;
}

LinkInfo ClassifyLink(const std::wstring& raw) {
    LinkInfo li;
    std::wstring u = Trim(raw);
    if (u.empty()) return li;
    std::wstring low = Lower(u);
    if (!StartsWith(low, L"http://") && !StartsWith(low, L"https://")) {
        u = L"https://" + u;
        low = Lower(u);
    }
    li.url = u;
    size_t hs = low.find(L"://") + 3;
    size_t pe = low.find_first_of(L"/?#", hs);
    std::wstring host = low.substr(hs, pe == std::wstring::npos ? std::wstring::npos : pe - hs);
    size_t colon = host.find(L':');
    if (colon != std::wstring::npos) host.resize(colon);
    std::wstring rest = pe == std::wstring::npos ? L"" : u.substr(pe);
    size_t hash = rest.find(L'#');
    if (hash != std::wstring::npos) rest.resize(hash);
    std::wstring path = rest, query;
    size_t q = rest.find(L'?');
    if (q != std::wstring::npos) {
        path = rest.substr(0, q);
        query = rest.substr(q + 1);
    }
    for (const wchar_t* p : {L"www.", L"m.", L"music."})
        if (StartsWith(host, p)) host = host.substr(wcslen(p));
    std::vector<std::wstring> segs;
    for (auto& s : Split(path, L'/'))
        if (!s.empty()) segs.push_back(s);

    if (host == L"youtu.be") {
        if (!segs.empty()) li.vid = segs[0];
        li.list = QueryParam(query, L"list");
        li.type = LINK_VIDEO;
    } else if (host == L"youtube.com" || host == L"youtube-nocookie.com") {
        std::wstring s0 = segs.empty() ? L"" : Lower(segs[0]);
        if (s0 == L"watch") {
            li.vid = QueryParam(query, L"v");
            li.list = QueryParam(query, L"list");
            if (!li.vid.empty()) li.type = LINK_VIDEO;
            else if (!li.list.empty()) {
                li.type = LINK_PLAYLIST;
                li.url = L"https://www.youtube.com/playlist?list=" + li.list;
            }
        } else if ((s0 == L"shorts" || s0 == L"live" || s0 == L"embed" || s0 == L"v") && segs.size() >= 2) {
            li.vid = segs[1];
            li.list = QueryParam(query, L"list");
            li.type = LINK_VIDEO;
        } else if (s0 == L"playlist") {
            li.list = QueryParam(query, L"list");
            li.type = LINK_PLAYLIST;
        } else if (!s0.empty() && (s0[0] == L'@' || s0 == L"channel" || s0 == L"c" || s0 == L"user")) {
            li.type = LINK_CHANNEL;
            size_t baseSegs = (s0[0] == L'@') ? 1 : 2;
            if (segs.size() <= baseSegs) {
                std::wstring base = L"https://www.youtube.com";
                for (auto& s : segs) base += L"/" + s;
                li.url = base + L"/videos";
            }
        }
    }
    if (!li.vid.empty() && !ValidId(li.vid)) li.vid.clear();
    return li;
}

std::vector<std::wstring> ExtractLinks(const std::wstring& text) {
    std::vector<std::wstring> out;
    std::set<std::wstring> seen;
    std::wstring cur;
    auto flush = [&] {
        std::wstring t = Trim(cur);
        cur.clear();
        while (!t.empty() && wcschr(L").,;'\"]}>", t.back())) t.pop_back();
        while (!t.empty() && wcschr(L"(\"'[{<", t.front())) t.erase(0, 1);
        if (t.empty()) return;
        std::wstring l = Lower(t);
        bool ok = StartsWith(l, L"http://") || StartsWith(l, L"https://") || l.find(L"youtube.com/") != std::wstring::npos ||
                  l.find(L"youtu.be/") != std::wstring::npos;
        if (ok && seen.insert(t).second) out.push_back(t);
    };
    for (wchar_t c : text) {
        if (iswspace(c)) flush();
        else cur += c;
    }
    flush();
    return out;
}

std::wstring FriendlyError(const std::wstring& raw) {
    std::wstring msg;
    for (auto& line : Split(raw, L'\n')) {
        std::wstring t = Trim(line);
        if (StartsWith(t, L"ERROR:")) msg = Trim(t.substr(6));
    }
    if (msg.empty()) {
        for (auto& line : Split(raw, L'\n')) {
            std::wstring t = Trim(line);
            if (!t.empty() && !StartsWith(t, L"WARNING")) msg = t;
        }
    }
    struct Rule {
        const wchar_t* needle;
        int sid;
    };
    static const Rule rules[] = {
        {L"Private video", S_ERR_PRIVATE},
        {L"members-only", S_ERR_MEMBERS},
        {L"Join this channel", S_ERR_MEMBERS},
        {L"confirm your age", S_ERR_AGE},
        {L"age-restricted", S_ERR_AGE},
        {L"not a bot", S_ERR_BOT},
        {L"Sign in to confirm", S_ERR_BOT},
        {L"Video unavailable", S_ERR_UNAVAILABLE},
        {L"This video is unavailable", S_ERR_UNAVAILABLE},
        {L"This live event will begin", S_ERR_LIVE},
        {L"Premieres in", S_ERR_PREMIERE},
        {L"Unsupported URL", S_ERR_UNSUPPORTED},
        {L"is not a valid URL", S_ERR_BADURL},
        {L"HTTP Error 429", S_ERR_429},
        {L"HTTP Error 403", S_ERR_403},
        {L"ffmpeg not found", S_ERR_FFMPEG_MISSING},
        {L"ffprobe and ffmpeg not found", S_ERR_FFMPEG_MISSING},
        {L"Requested format is not available", S_ERR_FORMAT},
        {L"No space left", S_ERR_DISK},
        {L"Permission denied", S_ERR_PERM},
        {L"getaddrinfo", S_ERR_NET},
        {L"Unable to download webpage", S_ERR_NET},
        {L"timed out", S_ERR_TIMEOUT},
    };
    for (auto& r : rules)
        if (msg.find(r.needle) != std::wstring::npos) return T(r.sid);
    if (msg.empty()) return T(S_ERR_UNKNOWN);
    if (msg[0] == L'[') {  // strip "[youtube] xxxx: " prefixes
        size_t p = msg.find(L"] ");
        if (p != std::wstring::npos) msg = msg.substr(p + 2);
        size_t c = msg.find(L": ");
        if (c != std::wstring::npos && c <= 20) msg = msg.substr(c + 2);
    }
    if (msg.size() > 220) msg = msg.substr(0, 220) + L"…";
    return msg;
}

// ============================================================ yt-dlp arguments

DlOptions OptionsFromBar(int forceKind) {
    DlOptions o;
    o.kind = forceKind >= 0 ? forceKind : g_set.kind;
    o.height = _wtoi(kVideoQuality[g_set.vq].value);
    o.aq = kAudioQuality[g_set.aq].value;
    o.vfmt = kVideoFormat[g_set.vfmt].value;
    o.afmt = kAudioFormat[g_set.afmt].value;
    o.sub = g_set.subLang;
    o.subAuto = g_set.autoSubs;
    o.alang = g_set.audioLang;
    o.dir = o.kind == KIND_VIDEO ? g_set.dirVideo : g_set.dirAudio;
    return o;
}

std::wstring OptSummary(const DlOptions& o) {
    std::wstring s;
    if (o.kind == KIND_VIDEO) {
        s = (o.height > 0 ? Fmt(L"%dp", o.height) : std::wstring(T(S_Q_BEST_VIDEO))) + L" · " + Upper(o.vfmt);
    } else {
        s = Upper(o.afmt);
        if (o.afmt == L"mp3")
            s += (o.aq.empty() || o.aq == L"0") ? std::wstring(L" · ") + T(S_Q_BEST_AUDIO)
                                                : L" · " + o.aq.substr(0, o.aq.size() - 1) + L" kbps";
    }
    if (!o.sub.empty())
        s += std::wstring(L" · ") + T(S_LBL_SUBS) + L" " + (o.sub == L"all" ? std::wstring(T(S_SUB_ALL)) : NativeLangName(o.sub.c_str()));
    if (!o.alang.empty()) s += std::wstring(L" · ") + T(S_LBL_AUDIO) + L" " + NativeLangName(o.alang.c_str());
    return s;
}

// warnings: downloads keep them, because a refused subtitle only ever shows up
// as a warning and the app needs to see it to fetch the subtitle again later.
static std::wstring CommonArgs(const Tools& t, bool warnings = false) {
    std::wstring a = L"--ignore-config --no-update";
    if (!warnings) a += L" --no-warnings";
    if (!t.qjs.empty()) a += L" --js-runtimes " + QuoteArg(L"quickjs:" + t.qjs);
    if (!t.ffmpegDir.empty()) a += L" --ffmpeg-location " + QuoteArg(t.ffmpegDir);
    std::wstring proxy = Trim(g_set.proxy);
    if (g_set.useProxy && !proxy.empty()) a += L" --proxy " + QuoteArg(proxy);
    if (!g_set.cookieFile.empty() && FileExists(g_set.cookieFile)) a += L" --cookies " + QuoteArg(g_set.cookieFile);
    else if (g_set.browser > 0) a += L" --cookies-from-browser " + std::wstring(kBrowsers[g_set.browser].value);
    return a;
}

std::wstring AnalyzeArgs(const Tools& t, const std::wstring& url, bool single) {
    std::wstring a = CommonArgs(t) + L" -J --flat-playlist";
    if (single) a += L" --no-playlist";
    return a + L" " + QuoteArg(url);
}

// True when two downloads can share one yt-dlp process: everything that shapes
// the command line has to match, the URL aside.
bool SameDownloadOptions(const DlOptions& a, const DlOptions& b) {
    return a.kind == b.kind && a.height == b.height && a.aq == b.aq && a.vfmt == b.vfmt && a.afmt == b.afmt &&
           a.sub == b.sub && a.subAuto == b.subAuto && a.alang == b.alang && a.dir == b.dir;
}

std::wstring SubJobDir() { return PathJoin(LocalDataDir(), L"subtitle-jobs"); }

// Text that must reach yt-dlp's output template literally: % starts a field.
static std::wstring TemplateLiteral(const std::wstring& s) {
    std::wstring r;
    for (wchar_t c : s) r += c == L'%' ? std::wstring(L"%%") : std::wstring(1, c);
    return r;
}

std::wstring DownloadArgs(const Tools& t, const Item& it) {
    const DlOptions& o = it.opt;
    std::wstring a = CommonArgs(t, true);
    // Without -i a refused subtitle fails the whole video. YouTube refuses
    // machine-translated subtitles that come without a PO token, so a playlist
    // with translated subtitles turned on used to fail almost row by row.
    a += L" -i";
    int retries = kRetries[g_set.retries];
    a += Fmt(L" -R %d --fragment-retries %d", retries, retries);
    a += L" -q --progress --newline --progress-delta 0.3 --no-playlist";
    // The video id leads every line so one process can carry several downloads
    // and the list still knows which row each line belongs to.
    a += L" --progress-template " +
         QuoteArg(L"download:[P]%(info.id)s|%(progress.downloaded_bytes)s|%(progress.total_bytes)s|"
                  L"%(progress.total_bytes_estimate)s|%(progress.speed)s|%(progress.eta)s|%(info.vcodec)s");
    a += L" --print " + QuoteArg(L"after_move:[DONE]%(id)s|%(height)s|%(fps)s|%(abr)s|%(ext)s|%(filepath)s");
    std::wstring rate = kRateLimits[g_set.rate].value;
    if (!rate.empty()) a += L" -r " + rate;
    // YouTube serves a stream at roughly 20 Mbit/s per connection. Fetching it
    // in pieces over several connections at once is what makes other download
    // managers fast: measured here, a 1080p stream went from 8 to 28 MiB/s.
    // Fewer pieces each when more downloads run, about sixteen connections all told.
    int pieces = std::max(2, std::min(8, 16 / std::max(1, g_set.concurrency)));
    a += Fmt(L" --extractor-args youtube:formats=dashy -N %d", pieces);

    std::wstring langSel = o.alang.empty() ? L"" : L"[language^=" + o.alang + L"]";
    if (o.kind == KIND_VIDEO) {
        std::wstring f = o.alang.empty() ? L"bv+ba/b" : L"bv+ba" + langSel + L"/bv+ba/b";
        std::wstring sort = o.height > 0 ? Fmt(L"res:%d", o.height) : L"res";
        sort += g_set.fps == 1 ? L",fps:30" : L",fps";
        std::wstring codec = kCodecs[g_set.codec].value;
        sort += codec == L"avc" ? std::wstring(L",+codec:avc:m4a") : L",vcodec:" + codec;
        a += L" -f " + QuoteArg(f) + L" -S " + QuoteArg(sort) + L" --merge-output-format " + o.vfmt;
        if (g_set.embedMeta) a += L" --embed-metadata --embed-thumbnail --embed-chapters";
        if (!o.sub.empty()) {
            a += L" --write-subs";
            if (o.sub == L"all") a += L" --sub-langs \"all,-live_chat\"";
            else {
                if (o.subAuto) a += L" --write-auto-subs";
                a += L" --sub-langs " + QuoteArg(o.sub);
            }
            a += L" --embed-subs";
        }
    } else {
        std::wstring base = o.afmt == L"m4a" ? L"ba[ext=m4a]" : o.afmt == L"opus" ? L"ba[acodec=opus]" : L"ba";
        std::wstring f;
        if (!o.alang.empty()) f = base + langSel + L"/ba" + langSel + L"/";
        f += base + L"/ba/b";
        a += L" -f " + QuoteArg(f) + L" -x --audio-format " + o.afmt;
        if (o.afmt == L"mp3") a += L" --audio-quality " + (o.aq.empty() ? std::wstring(L"0") : o.aq);
        if (g_set.embedMeta) a += L" --embed-metadata --embed-thumbnail";
        if (!o.sub.empty()) {
            a += L" --write-subs";
            if (o.sub == L"all") a += L" --sub-langs \"all,-live_chat\"";
            else {
                if (o.subAuto) a += L" --write-auto-subs";
                a += L" --sub-langs " + QuoteArg(o.sub);
            }
            a += L" --convert-subs srt";
        }
    }
    std::wstring dir = o.dir.empty() ? KnownFolder(FOLDERID_Downloads) : o.dir;
    a += L" -P " + QuoteArg(L"home:" + dir);
    a += L" -o " + QuoteArg(std::wstring(kNameTemplates[g_set.nameTpl].value) + L".%(ext)s");
    // Keep what yt-dlp learned about the video, so a refused subtitle can be
    // asked for again later without analysing the video a second time.
    if (!o.sub.empty()) a += L" -o " + QuoteArg(L"infojson:" + TemplateLiteral(SubJobDir()) + L"\\%(id)s");
    if (!o.sub.empty()) a += L" --write-info-json";
    a += L" " + QuoteArg(it.url);
    return a;
}

// Several videos through one process. yt-dlp fetches the YouTube player once
// and reuses it for the rest, which saves both time and requests to YouTube.
std::wstring DownloadArgs(const Tools& t, const std::vector<const Item*>& batch) {
    if (batch.empty()) return L"";
    std::wstring a = DownloadArgs(t, *batch.front());
    for (size_t i = 1; i < batch.size(); ++i) a += L" " + QuoteArg(batch[i]->url);
    return a;
}

// Fetches only the subtitles of a finished download, from the information saved
// while it downloaded. No new analysis, so this costs YouTube a single request.
// The result is a .srt beside the video, which players pick up by name.
std::wstring SubtitleArgs(const Tools& t, const Item& it, const std::wstring& infoJson) {
    const DlOptions& o = it.opt;
    std::wstring a = CommonArgs(t, true);
    a += L" --load-info-json " + QuoteArg(infoJson) + L" --skip-download --write-subs";
    if (o.sub == L"all") a += L" --sub-langs \"all,-live_chat\"";
    else {
        if (o.subAuto) a += L" --write-auto-subs";
        a += L" --sub-langs " + QuoteArg(o.sub);
    }
    a += L" --convert-subs srt";
    a += L" -P " + QuoteArg(L"home:" + PathDir(it.file));
    a += L" -o " + QuoteArg(TemplateLiteral(PathStem(it.file)) + L".%(ext)s");
    return a;
}

std::wstring BuildInfo(const DlOptions& o, const std::wstring& height, const std::wstring& fps, const std::wstring& abr,
                       const std::wstring& ext) {
    std::wstring e = Upper(ext == L"NA" ? std::wstring() : ext);
    if (e.empty()) e = Upper(o.kind == KIND_VIDEO ? o.vfmt : o.afmt);
    if (o.kind == KIND_VIDEO) {
        int h = _wtoi(height.c_str());
        int f = (int)(_wtof(fps.c_str()) + 0.5);
        if (h <= 0) return e;
        return Fmt(L"%dp", h) + (f > 30 ? std::to_wstring(f) : L"") + L" · " + e;
    }
    if (o.afmt == L"mp3") {
        if (o.aq.empty() || o.aq == L"0") return e + L" · VBR";
        return e + L" · " + o.aq.substr(0, o.aq.size() - 1) + L" kbps";
    }
    double ab = _wtof(abr.c_str());
    return ab > 0 ? e + Fmt(L" · %.0f kbps", ab) : e;
}

// ============================================================ metadata

static std::wstring JW(const JValue& v, std::initializer_list<const char*> keys) {
    for (auto k : keys) {
        std::string s = v[k].str();
        if (!s.empty()) return W(s);
    }
    return L"";
}

static int CodecRank(const std::string& vc) {
    auto fam = [](const std::string& c) {
        if (c.rfind("avc", 0) == 0 || c.rfind("h264", 0) == 0) return 0;
        if (c.rfind("vp09", 0) == 0 || c.rfind("vp9", 0) == 0) return 1;
        if (c.rfind("av01", 0) == 0) return 2;
        return 3;
    };
    std::wstring pref = kCodecs[g_set.codec].value;
    int p = pref == L"avc" ? 0 : pref == L"vp9" ? 1 : 2;
    int f = fam(vc);
    return f == p ? 0 : 1 + f;
}

bool ParseMeta(const std::string& out, const LinkInfo& link, VideoMeta& m) {
    std::string json;
    size_t pos = 0;
    while (pos < out.size()) {
        size_t e = out.find('\n', pos);
        std::string line = out.substr(pos, e == std::string::npos ? std::string::npos : e - pos);
        if (!line.empty() && line[0] == '{') {
            json = line;
            break;
        }
        if (e == std::string::npos) break;
        pos = e + 1;
    }
    if (json.empty()) return false;
    JValue j;
    if (!JParser(json).parse(j) || j.t != JValue::Obj) return false;

    m.url = JW(j, {"webpage_url", "original_url"});
    if (m.url.empty()) m.url = link.url;
    m.title = JW(j, {"title"});
    m.channel = JW(j, {"channel", "uploader"});
    m.channelUrl = JW(j, {"channel_url", "uploader_url"});
    std::string type = j["_type"].str("video");

    if (type == "playlist") {
        m.isGroup = true;
        m.groupType = link.type == LINK_CHANNEL ? GROUP_CHANNEL : GROUP_PLAYLIST;
        if (m.groupType == GROUP_CHANNEL && !m.channel.empty()) m.title = m.channel;
        if (m.title.empty()) m.title = link.url;
        for (auto& e : j["entries"].a) {
            if (e.t != JValue::Obj) continue;
            if (e["ie_key"].str() == "YoutubeTab") continue;
            EntryInfo en;
            en.vid = JW(e, {"id"});
            en.url = JW(e, {"url", "webpage_url"});
            if (en.url.empty() && !en.vid.empty()) en.url = L"https://www.youtube.com/watch?v=" + en.vid;
            if (en.url.empty()) continue;
            en.title = JW(e, {"title"});
            if (en.title == L"[Private video]" || en.title == L"[Deleted video]") continue;
            if (en.title.empty()) en.title = en.url;
            en.channel = JW(e, {"channel", "uploader"});
            if (en.channel.empty()) en.channel = m.channel;
            en.channelUrl = JW(e, {"channel_url", "uploader_url"});
            if (en.channelUrl.empty()) en.channelUrl = m.channelUrl;
            en.duration = e["duration"].num(0);
            if (!ValidId(en.vid)) en.vid.clear();
            m.entries.push_back(en);
        }
        if (!m.entries.empty()) m.vid = m.entries[0].vid;
        return true;
    }

    m.vid = JW(j, {"id"});
    if (!ValidId(m.vid)) m.vid = link.vid;
    m.duration = j["duration"].num(0);
    m.uploadDate = JW(j, {"upload_date"});
    std::map<int, QualityInfo> byHeight;
    std::map<std::wstring, bool> langs;
    for (auto& f : j["formats"].a) {
        std::string vc = f["vcodec"].str("none"), ac = f["acodec"].str("none");
        std::string ext = f["ext"].str(), proto = f["protocol"].str();
        if (ext == "mhtml" || proto == "mhtml") continue;
        long long size = (long long)f["filesize"].num(0);
        if (size <= 0) size = (long long)f["filesize_approx"].num(0);
        if (!vc.empty() && vc != "none") {
            int h = (int)f["height"].num(0);
            if (h <= 0) continue;
            int fps = (int)(f["fps"].num(0) + 0.5);
            int rank = CodecRank(vc);
            QualityInfo& q = byHeight[h];
            if (q.height == 0 || rank < q.rank || (rank == q.rank && size > q.size)) {
                q.height = h;
                q.rank = rank;
                q.size = size;
            }
            q.fps = std::max(q.fps, fps);
        } else if (!ac.empty() && ac != "none") {
            double abr = f["abr"].num(0);
            if (abr > m.audioAbr) {
                m.audioAbr = abr;
                m.audioSize = size;
            }
            std::wstring lang = JW(f, {"language"});
            std::string note = f["format_note"].str();
            if (!lang.empty()) langs[lang] = langs[lang] || note.find("original") != std::string::npos;
        }
    }
    for (auto it = byHeight.rbegin(); it != byHeight.rend(); ++it) m.qualities.push_back(it->second);
    if (langs.size() > 1)
        for (auto& kv : langs) {
            std::wstring name = NativeLangName(kv.first.c_str());
            m.audioLangs.push_back({kv.first, kv.second ? Fmt(T(S_OPT_ORIG_TRACK), name.c_str()) : name});
        }

    std::set<std::wstring> manual;
    for (auto& kv : j["subtitles"].o) {
        std::wstring code = W(kv.first);
        if (code == L"live_chat") continue;
        manual.insert(code);
        m.subs.push_back({code, NativeLangName(code.c_str()), false});
    }
    for (auto& kv : j["automatic_captions"].o) {
        std::wstring code = W(kv.first);
        size_t p = code.find(L"-orig");
        if (p != std::wstring::npos && p + 5 == code.size()) {
            std::wstring base = code.substr(0, p);
            m.subs.push_back({code, Fmt(T(S_OPT_AUTOSUB), NativeLangName(base.c_str())), true});
        }
    }
    // offer auto-translated captions in the UI language and in English
    for (const wchar_t* c : {kLangs[g_lang].code, L"en"}) {
        if (manual.count(c)) continue;
        bool already = false;
        for (auto& s : m.subs)
            if (s.code == c) already = true;
        if (!already && j["automatic_captions"].find(U8(c).c_str()))
            m.subs.push_back({c, Fmt(T(S_OPT_AUTOTRANS), NativeLangName(c)), true});
    }
    return true;
}

// ============================================================ history

static std::wstring HistoryPath() { return PathJoin(LocalDataDir(), L"history.tsv"); }

static std::wstring Esc(const std::wstring& s) {
    std::wstring r;
    for (wchar_t c : s) {
        if (c == L'\\') r += L"\\\\";
        else if (c == L'\t') r += L"\\t";
        else if (c == L'\n') r += L"\\n";
        else if (c == L'\r') r += L"\\r";
        else r += c;
    }
    return r;
}

static std::wstring Unesc(const std::wstring& s) {
    std::wstring r;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == L'\\' && i + 1 < s.size()) {
            wchar_t n = s[++i];
            r += n == L't' ? L'\t' : n == L'n' ? L'\n' : n == L'r' ? L'\r' : n;
        } else {
            r += s[i];
        }
    }
    return r;
}

void SaveHistory(const std::vector<std::unique_ptr<Item>>& items) {
    std::wstring out = L"YTPC1\n";
    for (auto& up : items) {
        const Item& it = *up;
        if (it.st == St::Analyzing || it.st == St::Choosing) continue;
        St st = it.st;
        if (st == St::Queued || st == St::Downloading || st == St::Processing) st = St::Canceled;
        std::vector<std::wstring> f = {
            it.url, it.vid, it.title, it.channel, it.channelUrl, std::to_wstring(it.groupType), it.groupTitle, it.groupUrl,
            it.playlistUrl, Fmt(L"%.0f", it.duration), std::to_wstring((int)st), it.file, std::to_wstring(it.size), it.info,
            it.err, it.isNew ? L"1" : L"0", std::to_wstring(it.added), std::to_wstring(it.opt.kind),
            std::to_wstring(it.opt.height), it.opt.aq, it.opt.vfmt, it.opt.afmt, it.opt.sub, it.opt.subAuto ? L"1" : L"0",
            it.opt.alang, it.opt.dir,
        };
        std::wstring line;
        for (size_t i = 0; i < f.size(); ++i) {
            if (i) line += L'\t';
            line += Esc(f[i]);
        }
        out += line + L"\n";
    }
    EnsureDir(LocalDataDir());
    WriteFileBytes(HistoryPath(), U8(out));
}

void LoadHistory(std::vector<std::unique_ptr<Item>>& items, int& nextId) {
    std::string raw;
    if (!ReadFileBytes(HistoryPath(), raw)) return;
    auto lines = Split(W(raw), L'\n');
    if (lines.empty() || Trim(lines[0]) != L"YTPC1") return;
    for (size_t i = 1; i < lines.size(); ++i) {
        std::wstring line = lines[i];
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        if (line.empty()) continue;
        auto f = Split(line, L'\t');
        if (f.size() < 26) continue;
        for (auto& x : f) x = Unesc(x);
        auto it = std::make_unique<Item>();
        it->id = nextId++;
        it->url = f[0];
        it->vid = f[1];
        it->title = f[2];
        it->channel = f[3];
        it->channelUrl = f[4];
        it->groupType = _wtoi(f[5].c_str());
        it->groupTitle = f[6];
        it->groupUrl = f[7];
        it->playlistUrl = f[8];
        it->duration = _wtof(f[9].c_str());
        int st = _wtoi(f[10].c_str());
        it->st = (st == (int)St::Done || st == (int)St::Error) ? (St)st : St::Canceled;
        it->file = f[11];
        it->size = _wtoi64(f[12].c_str());
        it->info = f[13];
        it->err = f[14];
        it->isNew = f[15] == L"1";
        it->added = _wtoi64(f[16].c_str());
        it->opt.kind = _wtoi(f[17].c_str()) == 1 ? KIND_AUDIO : KIND_VIDEO;
        it->opt.height = _wtoi(f[18].c_str());
        it->opt.aq = f[19];
        it->opt.vfmt = f[20].empty() ? L"mp4" : f[20];
        it->opt.afmt = f[21].empty() ? L"mp3" : f[21];
        it->opt.sub = f[22];
        it->opt.subAuto = f[23] == L"1";
        it->opt.alang = f[24];
        it->opt.dir = f[25];
        if (it->url.empty()) continue;
        items.push_back(std::move(it));
    }
}
