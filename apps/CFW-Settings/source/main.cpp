#include <string.h>
#include <stdio.h>
#include <chrono>

#include <switch.h>

#include "gui.hpp"
#include "gui_main.hpp"

#include "threads.hpp"
#include "title.hpp"

static Gui *currGui = nullptr;
static bool updateThreadRunning = false;
static Mutex mutexCurrGui;
u32 __nx_applet_type = AppletType_Default;

void update(void *args) {
  while (updateThreadRunning) {
    auto begin = std::chrono::steady_clock::now();

    mutexLock(&mutexCurrGui);
    if (currGui != nullptr)
      currGui->update();

    mutexUnlock(&mutexCurrGui);

    svcSleepThread(1.0E6 - std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - begin).count());
  }
}

int main(int argc, char **argv){
    u64 kdown = 0;
    touchPosition touch;
    u8 touchCntOld = 0, touchCnt = 0;

    gfxInitDefault();

    socketInitializeDefault();
    nxlinkStdio();

    setsysInitialize();
    ColorSetId colorSetId;
    setsysGetColorSetId(&colorSetId);
    setTheme(colorSetId);
    setsysExit();

    std::vector<FsSaveDataInfo> saveInfoList;
    Title::getSaveList(saveInfoList);

    for (auto saveInfo : saveInfoList) {
      if (Title::g_titles.find(saveInfo.titleID) == Title::g_titles.end())
        Title::g_titles.insert({(u64)saveInfo.titleID, new Title(saveInfo)});
    }

    Gui::g_nextGui = GUI_MAIN;

    mutexInit(&mutexCurrGui);

    updateThreadRunning = true;
    Threads::create(&update);

    touchCntOld = hidTouchCount();

    while(appletMainLoop()) {
      hidScanInput();
      kdown = hidKeysDown(CONTROLLER_P1_AUTO);

      if (Gui::g_nextGui != GUI_INVALID) {
        mutexLock(&mutexCurrGui);
        switch(Gui::g_nextGui) {
          case GUI_MAIN:
            currGui = new GuiMain();
            break;
        }
        mutexUnlock(&mutexCurrGui);
        Gui::g_nextGui = GUI_INVALID;
      }

      if(currGui != nullptr) {
        currGui->draw();

        if (kdown) {
          if(Gui::g_currListSelector != nullptr)
            Gui::g_currListSelector->onInput(kdown);
          else currGui->onInput(kdown);
        }

        touchCnt = hidTouchCount();

        if (touchCnt > touchCntOld) {
          hidTouchRead(&touch, 0);
          if(Gui::g_currListSelector != nullptr)
            Gui::g_currListSelector->onTouch(touch);
          else currGui->onTouch(touch);
        }

        touchCntOld = touchCnt;
      }
    }


    updateThreadRunning = false;
    Threads::joinAll();

    socketExit();

    gfxExit();
    return 0;
}
