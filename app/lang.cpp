#include "lang.h"
#include <cwchar>

const LangInfo kLangs[LANG_COUNT] = {
    {L"ko", L"한국어", L"Malgun Gothic", LANG_KOREAN},
    {L"tr", L"Türkçe", L"Segoe UI", LANG_TURKISH},
    {L"mn", L"Монгол", L"Segoe UI", LANG_MONGOLIAN},
    {L"en", L"English", L"Segoe UI", LANG_ENGLISH},
    {L"zh-Hans", L"简体中文", L"Microsoft YaHei UI", LANG_CHINESE_SIMPLIFIED},
    {L"zh-Hant", L"繁體中文", L"Microsoft JhengHei UI", LANG_CHINESE_TRADITIONAL},
    {L"es", L"Español", L"Segoe UI", LANG_SPANISH},
    {L"fr", L"Français", L"Segoe UI", LANG_FRENCH},
    {L"de", L"Deutsch", L"Segoe UI", LANG_GERMAN},
    {L"pt", L"Português", L"Segoe UI", LANG_PORTUGUESE},
    {L"ru", L"Русский", L"Segoe UI", LANG_RUSSIAN},
    {L"vi", L"Tiếng Việt", L"Segoe UI", LANG_VIETNAMESE},
    {L"th", L"ไทย", L"Leelawadee UI", LANG_THAI},
    {L"id", L"Bahasa Indonesia", L"Segoe UI", LANG_INDONESIAN},
    {L"ja", L"日本語", L"Yu Gothic UI", LANG_JAPANESE},
};

// strings.inc holds 13 columns in its own historical order; Mongolian and Turkish
// sit in a second table. kColumn maps a display language to where its text lives.
constexpr int kBaseColumns = 13;
static const wchar_t* const kTable[][kBaseColumns] = {
#define X(id, ...) {__VA_ARGS__},
#include "strings.inc"
#undef X
};
static_assert(sizeof kTable / sizeof kTable[0] == S_COUNT, "strings.inc rows must match the Str enum");

static const wchar_t* const kTableMnTr[][2] = {
#define Y(id, mn, tr) {mn, tr},
#include "strings-mn-tr.inc"
#undef Y
};
static_assert(sizeof kTableMnTr / sizeof kTableMnTr[0] == S_COUNT, "strings-mn-tr.inc must have one row per string");
static_assert((int)S_COUNT_MNTR == (int)S_COUNT, "strings-mn-tr.inc must list the same ids as strings.inc");

// 0-12 index kTable, 13 is Mongolian and 14 is Turkish in kTableMnTr.
static const int kColumn[LANG_COUNT] = {
    0,   // ko
    14,  // tr
    13,  // mn
    1,   // en
    3,   // zh-Hans
    4,   // zh-Hant
    5,   // es
    6,   // fr
    7,   // de
    8,   // pt
    9,   // ru
    10,  // vi
    11,  // th
    12,  // id
    2,   // ja
};

static const wchar_t* Cell(int id, int lang) {
    int col = kColumn[lang];
    return col < kBaseColumns ? kTable[id][col] : kTableMnTr[id][col - kBaseColumns];
}

const wchar_t* T(int id) {
    if (id < 0 || id >= S_COUNT) return L"";
    int lang = (g_lang >= 0 && g_lang < LANG_COUNT) ? g_lang : LANG_KO;
    const wchar_t* s = Cell(id, lang);
    return (s && *s) ? s : Cell(id, LANG_EN);
}

int LangFromCode(const wchar_t* code) {
    if (!code || !*code) return -1;
    for (int i = 0; i < LANG_COUNT; ++i)
        if (_wcsicmp(kLangs[i].code, code) == 0) return i;
    // accept plain "zh" and regional forms such as "zh-CN" / "pt-BR"
    if (_wcsnicmp(code, L"zh", 2) == 0) {
        if (wcsstr(code, L"TW") || wcsstr(code, L"HK") || wcsstr(code, L"Hant")) return LANG_ZH_HANT;
        return LANG_ZH_HANS;
    }
    for (int i = 0; i < LANG_COUNT; ++i)
        if (_wcsnicmp(kLangs[i].code, code, wcslen(kLangs[i].code)) == 0) return i;
    return -1;
}

int LangFromSystem() {
    LANGID id = GetUserDefaultUILanguage();
    WORD primary = PRIMARYLANGID(id);
    if (primary == LANG_CHINESE) {
        WORD sub = SUBLANGID(id);
        return (sub == SUBLANG_CHINESE_TRADITIONAL || sub == SUBLANG_CHINESE_HONGKONG || sub == SUBLANG_CHINESE_MACAU)
                   ? LANG_ZH_HANT
                   : LANG_ZH_HANS;
    }
    for (int i = 0; i < LANG_COUNT; ++i)
        if (kLangs[i].primary == primary) return i;
    return LANG_KO;
}

// ---------------------------------------------------------------- subtitle languages
// Shown in their own script, like the language menus of other download tools.
namespace {
struct NativeName {
    const wchar_t* code;
    const wchar_t* name;
};
const NativeName kNative[] = {
    {L"af", L"Afrikaans"},        {L"am", L"አማርኛ"},            {L"ar", L"العربية"},
    {L"az", L"Azərbaycanca"},     {L"be", L"Беларуская"},      {L"bg", L"Български"},
    {L"bn", L"বাংলা"},              {L"bs", L"Bosanski"},        {L"ca", L"Català"},
    {L"cs", L"Čeština"},          {L"cy", L"Cymraeg"},         {L"da", L"Dansk"},
    {L"de", L"Deutsch"},          {L"el", L"Ελληνικά"},        {L"en", L"English"},
    {L"eo", L"Esperanto"},        {L"es", L"Español"},         {L"et", L"Eesti"},
    {L"eu", L"Euskara"},          {L"fa", L"فارسی"},            {L"fi", L"Suomi"},
    {L"fil", L"Filipino"},        {L"fr", L"Français"},        {L"ga", L"Gaeilge"},
    {L"gl", L"Galego"},           {L"gu", L"ગુજરાતી"},            {L"ha", L"Hausa"},
    {L"he", L"עברית"},             {L"hi", L"हिन्दी"},              {L"hr", L"Hrvatski"},
    {L"hu", L"Magyar"},           {L"hy", L"Հայերեն"},         {L"id", L"Bahasa Indonesia"},
    {L"is", L"Íslenska"},         {L"it", L"Italiano"},        {L"ja", L"日本語"},
    {L"jv", L"Basa Jawa"},        {L"ka", L"ქართული"},         {L"kk", L"Қазақша"},
    {L"km", L"ខ្មែរ"},              {L"kn", L"ಕನ್ನಡ"},            {L"ko", L"한국어"},
    {L"ky", L"Кыргызча"},         {L"lo", L"ລາວ"},             {L"lt", L"Lietuvių"},
    {L"lv", L"Latviešu"},         {L"mk", L"Македонски"},      {L"ml", L"മലയാളം"},
    {L"mn", L"Монгол"},           {L"mr", L"मराठी"},             {L"ms", L"Bahasa Melayu"},
    {L"mt", L"Malti"},            {L"my", L"မြန်မာ"},            {L"ne", L"नेपाली"},
    {L"nl", L"Nederlands"},       {L"no", L"Norsk"},           {L"pa", L"ਪੰਜਾਬੀ"},
    {L"pl", L"Polski"},           {L"ps", L"پښتو"},             {L"pt", L"Português"},
    {L"ro", L"Română"},           {L"ru", L"Русский"},         {L"si", L"සිංහල"},
    {L"sk", L"Slovenčina"},       {L"sl", L"Slovenščina"},     {L"so", L"Soomaali"},
    {L"sq", L"Shqip"},            {L"sr", L"Српски"},          {L"su", L"Basa Sunda"},
    {L"sv", L"Svenska"},          {L"sw", L"Kiswahili"},       {L"ta", L"தமிழ்"},
    {L"te", L"తెలుగు"},             {L"th", L"ไทย"},             {L"tr", L"Türkçe"},
    {L"uk", L"Українська"},       {L"ur", L"اردو"},             {L"uz", L"Oʻzbek"},
    {L"vi", L"Tiếng Việt"},       {L"yi", L"ייִדיש"},            {L"yo", L"Yorùbá"},
    {L"zh", L"中文"},             {L"zh-Hans", L"简体中文"},   {L"zh-Hant", L"繁體中文"},
    {L"zh-CN", L"简体中文"},      {L"zh-TW", L"繁體中文"},     {L"zh-HK", L"繁體中文"},
    {L"zu", L"isiZulu"},
};
}  // namespace

const wchar_t* NativeLangName(const wchar_t* code) {
    if (!code || !*code) return L"";
    for (auto& n : kNative)
        if (_wcsicmp(n.code, code) == 0) return n.name;
    // "en-GB" and friends fall back to the base language
    const wchar_t* dash = wcschr(code, L'-');
    if (dash) {
        size_t len = (size_t)(dash - code);
        for (auto& n : kNative)
            if (wcslen(n.code) == len && _wcsnicmp(n.code, code, len) == 0) return n.name;
    }
    return code;
}

// Offered in the subtitle picker: the most used languages first, then the rest alphabetically.
const wchar_t* const kSubtitleCodes[] = {
    L"ko", L"tr", L"mn", L"en", L"zh-Hans", L"zh-Hant", L"es", L"fr", L"de", L"pt", L"ru", L"vi", L"th", L"id", L"ja",
    L"af", L"am", L"ar", L"az", L"be", L"bg", L"bn", L"bs", L"ca", L"cs", L"cy", L"da", L"el", L"eo", L"et",
    L"eu", L"fa", L"fi", L"fil", L"ga", L"gl", L"gu", L"ha", L"he", L"hi", L"hr", L"hu", L"hy", L"is", L"it",
    L"jv", L"ka", L"kk", L"km", L"kn", L"ky", L"lo", L"lt", L"lv", L"mk", L"ml", L"mr", L"ms", L"mt",
    L"my", L"ne", L"nl", L"no", L"pa", L"pl", L"ps", L"ro", L"si", L"sk", L"sl", L"so", L"sq", L"sr", L"su",
    L"sv", L"sw", L"ta", L"te", L"uk", L"ur", L"uz", L"yi", L"yo", L"zu",
};
const int kSubtitleCodeCount = (int)(sizeof kSubtitleCodes / sizeof kSubtitleCodes[0]);
