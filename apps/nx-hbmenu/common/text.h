#pragma once
#include "common.h"
#include "language.h"

Result textInit(void);
int textGetLang(void);
const char* textGetString(StrId id);
u64 textGetLanguageCode(void);
