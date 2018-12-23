#pragma once

#include "common.h"


typedef struct
{
    // Mandatory fields
    const char* name;
    //u32 flags;
    bool (* init)(void);
    void (* deinit)(void);
    void (* launchFile)(const char* path, argData_s* args);

    // Optional fields
    //void (* useTitle)(u64 tid, u8 mediatype);
} loaderFuncs_s;

bool launchInit(void);
void launchExit(void);
const loaderFuncs_s* launchGetLoader(void);
size_t launchAddArg(argData_s* ad, const char* arg);
void launchAddArgsFromString(argData_s* ad, char* arg);
void launchMenuEntry(menuEntry_s* me);
