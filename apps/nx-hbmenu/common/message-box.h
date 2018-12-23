#pragma once

typedef struct
{
    uint32_t width;
    uint32_t height;
    color_t *bg;
    char *text;
} MessageBox;

void menuCreateMsgBox(int width, int height, const char *text);
void menuCloseMsgBox();
bool menuIsMsgBoxOpen();
void menuDrawMsgBox(void);
MessageBox menuGetCurrentMsgBox();
void menuMsgBoxSetNetloaderState(bool enabled, const char *text, bool enable_progress, float progress);
