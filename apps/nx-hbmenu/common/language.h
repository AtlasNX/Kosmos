#pragma once
#ifdef __SWITCH__
#include <switch.h>
#endif

typedef enum
{
    StrId_Loading = 0,
    StrId_Directory,
    StrId_DefaultPublisher,
    StrId_IOError,
    StrId_CouldNotOpenFile,

    StrId_NoAppsFound_Title,
    StrId_NoAppsFound_Msg,

    StrId_LastLoadResult,
    StrId_AppLaunchError,

    StrId_AppInfo_Author,
    StrId_AppInfo_Version,
    StrId_Actions_Launch,
    StrId_Actions_Open,
    StrId_Actions_Back,
    StrId_Actions_Apply,

    StrId_MsgBox_OK,

    StrId_Reboot,
    StrId_ReturnToHome,

    StrId_TitleSelector,
    StrId_ErrorReadingTitleMetadata,
    StrId_NoTitlesFound,
    StrId_SelectTitle,

    StrId_NoTargetTitleSupport,
    StrId_MissingTargetTitle,

    StrId_NetLoader,
    StrId_NetLoaderUnavailable,
    StrId_NetLoaderOffline,
    StrId_NetLoaderError,
    StrId_NetLoaderActive,
    StrId_NetLoaderTransferring,

    StrId_ThemeMenu,
    StrId_ThemeNotApplied,
    StrId_DefaultThemeName,

    StrId_Max,
} StrId;

extern const char* const g_strings[StrId_Max][16];

