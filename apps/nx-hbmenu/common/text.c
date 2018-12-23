#include "text.h"

static u64 s_textLanguageCode = 0;

#ifdef __SWITCH__
static int s_textLang = SetLanguage_ENUS;
#else
static int s_textLang = 1;
#endif

Result textInit(void) {
    #ifdef __SWITCH__
    s32 Language=0;

    s_textLang = SetLanguage_ENUS;

    Result rc = setInitialize();
    if (R_SUCCEEDED(rc)) rc = setGetSystemLanguage(&s_textLanguageCode);
    if (R_SUCCEEDED(rc)) rc = setMakeLanguage(s_textLanguageCode, &Language);
    //if (R_SUCCEEDED(rc) && Language < 16) s_textLang = Language;//TODO: Re-enable this once language.c supports all used languages.
    setExit();
    if (R_FAILED(rc)) return rc;
    #else
    s_textLang = 1;
    #endif

    return 0;
}

int textGetLang(void) {
    return s_textLang;
}

const char* textGetString(StrId id) {
    const char* str = g_strings[id][s_textLang];
    #ifdef __SWITCH__
    if (!str) str = g_strings[id][SetLanguage_ENUS];
    #else                              
    if (!str) str = g_strings[id][1];
    #endif
    return str;
}

u64 textGetLanguageCode(void) {
    return s_textLanguageCode;
}
