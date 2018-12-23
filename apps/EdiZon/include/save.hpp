#pragma once

#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <switch.h>
#include "json.hpp"

#define SAVE_DEV "save"

Result _getSaveList(std::vector<FsSaveDataInfo> &saveInfoList);

s32 deleteDirRecursively(const char *path, bool isSave);
void makeExInjDir(char ptr[0x100], u64 titleID, u128 userID, bool isInject);
Result mountSaveByTitleAccountIDs(const u64 titleID, const u128 userID, FsFileSystem& tmpfs);
s32 isDirectory(const char *path);
s32 cpFile(std::string srcPath, std::string dstPath);
s32 copyAllSave(const char * path, bool isInject, const char exInjDir[0x100]);

s32 backupSave(u64 titleID, u128 userID, bool isBatch = false, time_t timestamp = time(nullptr));
s32 restoreSave(u64 titleID, u128 userID, const char* path);

s32 loadSaveFile(std::vector<u8> *buffer, size_t *length, u64 titleID, u128 userID, const char *path);
s32 storeSaveFile(u8 *buffer, size_t length, u64 titleID, u128 userID, const char *path);
