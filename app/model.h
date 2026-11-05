#pragma once
#include "common.h"

enum Kind { KIND_VIDEO = 0, KIND_AUDIO = 1 };
enum LinkType { LINK_OTHER = 0, LINK_VIDEO, LINK_PLAYLIST, LINK_CHANNEL };
enum GroupType { GROUP_NONE = 0, GROUP_PLAYLIST = 1, GROUP_CHANNEL = 2 };

// A dropdown entry. Labels are either a translated string id or a fixed
// technical term such as "1080p" or "MP4".
struct Choice {
    int sid;                // Str id, or -1 to use `fixed`
    const wchar_t* fixed;
    const wchar_t* value;
    std::wstring label() const;
};
extern const std::vector<Choice> kVideoQuality;
extern const std::vector<Choice> kAudioQuality;
extern const std::vector<Choice> kVideoFormat;
extern const std::vector<Choice> kAudioFormat;
extern const std::vector<Choice> kNameTemplates;
extern const std::vector<Choice> kCodecs;
extern const std::vector<Choice> kFps;
extern const std::vector<Choice> kRateLimits;
extern const std::vector<Choice> kBrowsers;
extern const std::vector<Choice> kNotifyWhen;
extern const std::vector<Choice> kThemes;
extern const int kRetries[4];
constexpr int kMaxConcurrent = 8;
std::vector<std::wstring> Labels(const std::vector<Choice>& c);
std::vector<std::wstring> LanguageLabels();   // native names for the language picker

// Subtitle / audio-track pickers, shown in each language's own script.
std::vector<std::wstring> SubtitleLabels();   // [none] + languages + [all]
std::wstring SubtitleCodeAt(int index);
int SubtitleIndexOf(const std::wstring& code);
std::vector<std::wstring> AudioLangLabels();  // [default] + languages
std::wstring AudioLangCodeAt(int index);
int AudioLangIndexOf(const std::wstring& code);

// Options used for one download
struct DlOptions {
    int kind = KIND_VIDEO;
    int height = 0;             // 0 = best
    std::wstring aq = L"0";     // mp3 quality ("0" = best VBR, "320K", ...)
    std::wstring vfmt = L"mp4";
    std::wstring afmt = L"mp3";
    std::wstring sub;           // subtitle language code, "" = none, "all"
    bool subAuto = true;
    std::wstring alang;         // audio track language, "" = default
    std::wstring dir;
};

struct Settings {
    // option bar
    int kind = 0, vq = 0, aq = 0, vfmt = 0, afmt = 0;
    std::wstring subLang;       // "" = none
    std::wstring audioLang;     // "" = default
    std::wstring dirVideo, dirAudio;
    bool quick = true;
    // appearance
    int theme = THEME_SYSTEM;
    int lang = LANG_KO;
    // general
    int nameTpl = 0;
    bool groupFolders = true;
    bool embedMeta = true;
    bool autoSubs = true;
    // download
    int concurrency = 3;  // simultaneous downloads, 1..kMaxConcurrent
    int fps = 0, codec = 0, rate = 0;
    // connection
    bool useProxy = false;
    std::wstring proxy;
    int retries = 2;  // index into kRetries
    // notification
    int notify = 1;  // 0 off, 1 when all done, 2 every item
    bool notifyErrors = true;
    bool sound = false;
    // login
    int browser = 0;
    std::wstring cookieFile;
    // tools
    std::wstring ytdlpPath, ffmpegPath;
    bool autoUpdate = true;
    long long lastUpdateCheck = 0;
    // window & view
    bool hasPos = false;
    int x = 0, y = 0, w = 0, h = 0;
    bool maximized = false;
    int tab = 0, sort = 0;
};
extern Settings g_set;
void LoadSettings();
void SaveSettings();

enum class St { Analyzing, Choosing, Queued, Downloading, Processing, Done, Error, Canceled };

struct Item {
    int id = 0;
    std::wstring url, vid, title, channel, channelUrl;
    int groupType = GROUP_NONE;
    std::wstring groupTitle, groupUrl;
    std::wstring playlistUrl;
    double duration = 0;
    DlOptions opt;
    St st = St::Queued;
    double pct = 0, speed = 0, eta = -1;
    long long done = 0, total = 0;
    int part = 0;
    bool gotDone = false;
    std::wstring lastError;
    std::wstring file, info, err;
    long long size = 0;
    bool isNew = false;
    long long added = 0;
    HBITMAP thumb = nullptr;
    bool thumbReq = false;
    int proc = 0;
    int autoRetry = 0;  // times this was requeued after YouTube rate-limited us
    int subState = 0;   // SUB_*: a subtitle YouTube refused while the video downloaded
    int subTries = 0;
    DWORD lastPaint = 0;
};

struct Tools {
    std::wstring ytdlp, ffmpegDir, ffmpegExe, qjs;
    bool ownCopy = false;  // yt-dlp lives in a folder we may update
};
Tools FindTools();
std::wstring UserToolsDir();

struct LinkInfo {
    int type = LINK_OTHER;
    std::wstring url, vid, list;
};
LinkInfo ClassifyLink(const std::wstring& raw);
std::vector<std::wstring> ExtractLinks(const std::wstring& text);
std::wstring FriendlyError(const std::wstring& raw);

DlOptions OptionsFromBar(int forceKind = -1);
std::wstring OptSummary(const DlOptions& o);
std::wstring AnalyzeArgs(const Tools& t, const std::wstring& url, bool single);
std::wstring DownloadArgs(const Tools& t, const Item& it);
std::wstring DownloadArgs(const Tools& t, const std::vector<const Item*>& batch);
bool SameDownloadOptions(const DlOptions& a, const DlOptions& b);

// Subtitles YouTube refused during a download are fetched again afterwards.
enum { SUB_OK = 0, SUB_WAIT, SUB_FAILED };
std::wstring SubJobDir();  // where the saved video information waits for that
std::wstring SubtitleArgs(const Tools& t, const Item& it, const std::wstring& infoJson);
std::wstring BuildInfo(const DlOptions& o, const std::wstring& height, const std::wstring& fps,
                       const std::wstring& abr, const std::wstring& ext);

struct SubTrack {
    std::wstring code, label;
    bool autoGen = false;
};
struct QualityInfo {
    int height = 0, fps = 0, rank = 99;
    long long size = 0;
};
struct EntryInfo {
    std::wstring url, vid, title, channel, channelUrl;
    double duration = 0;
    bool checked = true;
};
struct VideoMeta {
    bool isGroup = false;
    int groupType = GROUP_NONE;
    std::wstring url, vid, title, channel, channelUrl, uploadDate;
    double duration = 0;
    std::vector<QualityInfo> qualities;
    long long audioSize = 0;
    double audioAbr = 0;
    std::vector<SubTrack> subs;
    std::vector<std::pair<std::wstring, std::wstring>> audioLangs;
    std::vector<EntryInfo> entries;
};
bool ParseMeta(const std::string& output, const LinkInfo& link, VideoMeta& m);

void LoadHistory(std::vector<std::unique_ptr<Item>>& items, int& nextId);
void SaveHistory(const std::vector<std::unique_ptr<Item>>& items);

// menu command shared between the main window and the settings dialog
constexpr int IDM_UPDATE_YTDLP = 231;

// dialogs.cpp
bool ShowOptionsDialog(HWND owner, VideoMeta& meta, DlOptions& opt);
// Returns true when settings changed; uiChanged is set when theme or language changed.
bool ShowSettingsDialog(HWND owner, const Tools& tools, const std::wstring& ytdlpVersion, bool& uiChanged);
