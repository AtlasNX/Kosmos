#include "nx_touch.h"

#define TAP_MOVEMENT_GAP 20
#define VERTICAL_SWIPE_HORIZONTAL_PLAY 250
#define VERTICAL_SWIPE_MINIMUM_DISTANCE 300
#define HORIZONTAL_SWIPE_VERTICAL_PLAY 250
#define HORIZONTAL_SWIPE_MINIMUM_DISTANCE 300
#define LISTING_START_Y 475
#define LISTING_END_Y 647
#define BUTTON_START_Y 672
#define BUTTON_END_Y 704
#define BACK_BUTTON_START_X 966
#define BACK_BUTTON_END_X 1048
#define LAUNCH_BUTTON_START_X 1092
#define LAUNCH_BUTTON_END_X 1200

#define distance(x1, y1, x2, y2) (int) sqrt(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)))

struct touchInfo_s touchInfo;

void touchInit() {
    touchInfo.gestureInProgress = false;
    touchInfo.isTap = true;
    touchInfo.initMenuXPos = 0;
    touchInfo.initMenuIndex = 0;
}

void handleTappingOnApp(menu_s* menu, int px) {
    int i = 0;
    menuEntry_s *me = NULL;

    for (me = menu->firstEntry, i = 0; me; me = me->next, i ++) {
        int entry_start_x = 29 + i * (140 + 30);

        int screen_width = 1280;
        if (entry_start_x >= (screen_width - menu->xPos))
            break;

        if (px >= (entry_start_x + menu->xPos) && px <= (entry_start_x + menu->xPos) + 140 ) {
            launchMenuEntryTask(me);
            break;
        }
    }
}

void handleTappingOnOpenLaunch(menu_s* menu) {
    if (menu->nEntries > 0)
    {
        int i;
        menuEntry_s* me;
        for (i = 0, me = menu->firstEntry; i != menu->curEntry; i ++, me = me->next);
        launchMenuEntryTask(me);
    }
}

void handleTouch(menu_s* menu) {
    touchPosition currentTouch;
    u32 touches = hidTouchCount();

    // On touch start.
    if (touches == 1 && !touchInfo.gestureInProgress) {
        hidTouchRead(&currentTouch, 0);

        touchInfo.gestureInProgress = true;
        touchInfo.firstTouch = currentTouch;
        touchInfo.prevTouch = currentTouch;
        touchInfo.isTap = true;
        touchInfo.initMenuXPos = menu->xPos;
        touchInfo.initMenuIndex = menu->curEntry;
    }
    // On touch moving.
    else if (touches >= 1 && touchInfo.gestureInProgress) {
        hidTouchRead(&currentTouch, 0);

        touchInfo.prevTouch = currentTouch;

        if (touchInfo.isTap && (abs(touchInfo.firstTouch.px - currentTouch.px) > TAP_MOVEMENT_GAP || abs(touchInfo.firstTouch.py - currentTouch.py) > TAP_MOVEMENT_GAP)) {
            touchInfo.isTap = false;
        }
        if (!menuIsMsgBoxOpen() && touchInfo.firstTouch.py > LISTING_START_Y && touchInfo.firstTouch.py < LISTING_END_Y && !touchInfo.isTap && menu->nEntries > 7) {
            menu->xPos = touchInfo.initMenuXPos + (currentTouch.px - touchInfo.firstTouch.px);
            menu->curEntry = touchInfo.initMenuIndex + ((int) (touchInfo.firstTouch.px - currentTouch.px) / 170);

            if (menu->curEntry < 0)
                menu->curEntry = 0;

            if (menu->curEntry >= menu->nEntries - 6)
                menu->curEntry = menu->nEntries - 7;
        }
    }
    // On touch end.
    else if (touchInfo.gestureInProgress) {
        int x1 = touchInfo.firstTouch.px;
        int y1 = touchInfo.firstTouch.py;
        int x2 = touchInfo.prevTouch.px;
        int y2 = touchInfo.prevTouch.py;

        bool netloader_active = menuIsNetloaderActive();

        if (menuIsMsgBoxOpen() && !netloader_active) {
            MessageBox currMsgBox = menuGetCurrentMsgBox();
            int start_x = 1280 / 2 - currMsgBox.width / 2;
            int start_y = (720 / 2 - currMsgBox.height / 2) + (currMsgBox.height - 80);
            int end_x = start_x + currMsgBox.width;
            int end_y = start_y + 80;

            if (x1 > start_x && x1 < end_x && y1 > start_y && y1 < end_y && touchInfo.isTap) {
                menuCloseMsgBox();
            }
        } else if (touchInfo.isTap && !netloader_active) {
            // App Icons
            if (y1 > LISTING_START_Y && y1 < LISTING_END_Y) {
                handleTappingOnApp(menu, touchInfo.prevTouch.px);
            }
            // Bottom Buttons
            else if (y1 > BUTTON_START_Y && y1 < BUTTON_END_Y) {
                // Back Button for non-empty directory
                if (menu->nEntries != 0 && x1 > BACK_BUTTON_START_X && x1 < BACK_BUTTON_END_X) {
                    launchMenuBackTask();
                }
                // Open/Launch Button / Back Button for empty directories
                else if (x1 > LAUNCH_BUTTON_START_X && x1 < LAUNCH_BUTTON_END_X) {
                    if (menu->nEntries == 0) {
                        launchMenuBackTask();
                    } else {
                        handleTappingOnOpenLaunch(menu);
                    }
                }
            }
        }
        // Vertical Swipe
        else if (abs(x1 - x2) < VERTICAL_SWIPE_HORIZONTAL_PLAY && distance(x1, y1, x2, y2) > VERTICAL_SWIPE_MINIMUM_DISTANCE) {
            // Swipe up to go back
            if (y1 - y2 > 0) {
                launchMenuBackTask();
            }
            // Swipe down to go into netloader
            else if (y1 - y2 < 0) {
                launchMenuNetloaderTask();
            }
        }
        // Horizontal Swipe
        else if (y1 < LISTING_START_Y && y2 < LISTING_START_Y) {
            if (abs(y1 - y2) < HORIZONTAL_SWIPE_VERTICAL_PLAY && distance(x1, y1, x2, y2) > HORIZONTAL_SWIPE_MINIMUM_DISTANCE) {
                // Swipe left to go into theme-menu
                if (x1 - x2 > 0) {
                    themeMenuStartup();
                }
            }
        }

        touchInfo.gestureInProgress = false;
    }
}
