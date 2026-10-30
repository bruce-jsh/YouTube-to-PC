#pragma once
#include <windows.h>

// UI languages, in the order shown to the user.
// Display order: Korean first, then the two "brother countries", and Japanese last.
enum {
    LANG_KO = 0, LANG_TR, LANG_MN, LANG_EN, LANG_ZH_HANS, LANG_ZH_HANT, LANG_ES,
    LANG_FR, LANG_DE, LANG_PT, LANG_RU, LANG_VI, LANG_TH, LANG_ID, LANG_JA, LANG_COUNT
};

struct LangInfo {
    const wchar_t* code;    // settings value, e.g. "ko"
    const wchar_t* native;  // name shown in the picker, in its own script
    const wchar_t* font;    // UI font that covers the script
    WORD primary;           // PRIMARYLANGID for system-language detection
};
extern const LangInfo kLangs[LANG_COUNT];

int LangFromCode(const wchar_t* code);  // -1 when unknown
int LangFromSystem();                   // falls back to Korean

// String ids come from strings.inc so the table can never drift out of sync.
enum Str {
#define X(id, ...) id,
#include "strings.inc"
#undef X
    S_COUNT
};

// Mongolian and Turkish live in their own file; this checks it stays in step.
enum Str2 {
#define Y(id, ...) id##_MNTR,
#include "strings-mn-tr.inc"
#undef Y
    S_COUNT_MNTR
};

extern int g_lang;
const wchar_t* T(int id);

// Native name of a subtitle/audio language code ("ko" -> "한국어").
const wchar_t* NativeLangName(const wchar_t* code);
// Codes offered in the subtitle picker, ordered as shown.
extern const wchar_t* const kSubtitleCodes[];
extern const int kSubtitleCodeCount;
