#include <switch.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <chrono>

#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#include "gui.hpp"
#include "gui_main.hpp"
#include "gui_editor.hpp"
#include "gui_tx_warning.hpp"

#include "update_manager.hpp"

#include "title.hpp"

#include "threads.hpp"

extern "C" {
  #include "theme.h"
}

#define NXLINK

#define LONG_PRESS_DELAY              2
#define LONG_PRESS_ACTIVATION_DELAY   300

char* g_edizonPath;

static bool updateThreadRunning = false;
static Mutex mutexCurrGui;
static Gui* currGui = nullptr;
static u32 inputTicker = 0;
static u8 touchCntOld = 0, touchCnt = 0;
static u32 kheld = 0, kheldOld = 0;
static u32 kdown = 0;
static touchPosition touch;
static touchPosition touchEnd;

void initTitles() {
  std::vector<FsSaveDataInfo> saveInfoList;
  _getSaveList(saveInfoList);

  for (auto saveInfo : saveInfoList) {

    if (Title::g_titles.find(saveInfo.titleID) == Title::g_titles.end())
      Title::g_titles.insert({(u64)saveInfo.titleID, new Title(saveInfo)});

    Title::g_titles[saveInfo.titleID]->addUserID(saveInfo.userID);

    if (Account::g_accounts.find(saveInfo.userID) == Account::g_accounts.end())
      Account::g_accounts.insert({ static_cast<u128>(saveInfo.userID), new Account(saveInfo.userID) });
  }
}

void update(void *args) {
  while (updateThreadRunning) {
    auto begin = std::chrono::steady_clock::now();

    mutexLock(&mutexCurrGui);
    if (currGui != nullptr)
      currGui->update();
    mutexUnlock(&mutexCurrGui);

    if (kheld & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN)) inputTicker++;
    else inputTicker = 0;

    svcSleepThread(1.0E6 - std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - begin).count());
  }
}

int main(int argc, char** argv) {
  socketInitializeDefault();

#ifdef NXLINK
  nxlinkStdio();
#endif

  int file = open("/EdiZon/EdiZon.log", O_APPEND | O_WRONLY);

  if (file >= 0) {
    fflush(stdout);
    dup2(file, STDOUT_FILENO);
    fflush(stderr);
    dup2(file, STDERR_FILENO);
  }

  gfxInitDefault();

  setsysInitialize();
    ColorSetId colorSetId;
    setsysGetColorSetId(&colorSetId);
    setTheme(colorSetId);
  setsysExit();

  initTitles();

  Handle txHandle;
  if (R_FAILED(smRegisterService(&txHandle, "tx", false, 1)) && access("/EdiZon/.hide_sxos", F_OK) == -1)
    Gui::g_nextGui = GUI_TX_WARNING;
  else {
    Gui::g_nextGui = GUI_MAIN;
    smUnregisterService("tx");
  }

  g_edizonPath = new char[strlen(argv[0]) + 1];
  strcpy(g_edizonPath, argv[0] + 5);

  mutexInit(&mutexCurrGui);

  updateThreadRunning = true;
  Threads::create(&update);

  touchCntOld = hidTouchCount();

  while (appletMainLoop()) {
    hidScanInput();
    kheld = hidKeysHeld(CONTROLLER_P1_AUTO);
    kdown = hidKeysDown(CONTROLLER_P1_AUTO);

    if (Gui::g_nextGui != GUI_INVALID) {
      mutexLock(&mutexCurrGui);
      if (currGui != nullptr)
        delete currGui;

      switch (Gui::g_nextGui) {
        case GUI_MAIN:
          currGui = new GuiMain();
          break;
        case GUI_EDITOR:
          currGui = new GuiEditor();
          break;
        case GUI_TX_WARNING:
          currGui = new GuiTXWarning();
          break;
        default: break;
      }
      Gui::g_nextGui = GUI_INVALID;
      mutexUnlock(&mutexCurrGui);
    }

    if (currGui != nullptr) {
      currGui->draw();

      if (kdown || hidKeysUp(CONTROLLER_P1_AUTO)) {
        if (Gui::g_currMessageBox != nullptr)
          Gui::g_currMessageBox->onInput(kdown);
        else if (Gui::g_currListSelector != nullptr)
          Gui::g_currListSelector->onInput(kdown);
        else
          currGui->onInput(kdown);
      }

      if (inputTicker > LONG_PRESS_ACTIVATION_DELAY && (inputTicker % LONG_PRESS_DELAY) == 0) {
        if (Gui::g_currMessageBox != nullptr)
          Gui::g_currMessageBox->onInput(kheld);
        else if (Gui::g_currListSelector != nullptr)
          Gui::g_currListSelector->onInput(kheld);
        else
          currGui->onInput(kheld);
      }
    }

    touchCnt = hidTouchCount();

    if (touchCnt > touchCntOld)
      hidTouchRead(&touch, 0);

    if (touchCnt < touchCntOld) {
      if (Gui::g_currMessageBox != nullptr)
        Gui::g_currMessageBox->onTouch(touch);
      else if (Gui::g_currListSelector != nullptr)
        Gui::g_currListSelector->onTouch(touch);
      else {
        currGui->onTouch(touchEnd);
        currGui->onGesture(touch, touchEnd);
      }
    }

    hidTouchRead(&touchEnd, 0);

    touchCntOld = touchCnt;

    if (kheld != kheldOld)
      inputTicker = 0;

    kheldOld = kheld;

    if (kdown & KEY_PLUS)
      break;

    if (GuiMain::g_shouldUpdate) {
      Gui::g_currMessageBox->hide();

      UpdateManager updateManager;

      switch (updateManager.checkUpdate()) {
        case NONE: (new MessageBox("Latest configs and scripts are already installed!", MessageBox::OKAY))->show(); break;
        case ERROR: (new MessageBox("An error while downloading the updates has occured.", MessageBox::OKAY))->show(); break;
        case EDITOR: (new MessageBox("Updated editor configs and scripts to the latest version!", MessageBox::OKAY))->show(); break;
        case EDIZON: (new MessageBox("Updated EdiZon and editor configs and scripts to\nthe latest version! Please restart EdiZon!", MessageBox::OKAY))->show(); break;
      }

      GuiMain::g_shouldUpdate = false;
    }
  }

  updateThreadRunning = false;

  Threads::joinAll();

  delete[] g_edizonPath;

  for (auto it = Title::g_titles.begin(); it != Title::g_titles.end(); it++)
    delete it->second;

  for (auto it = Account::g_accounts.begin(); it != Account::g_accounts.end(); it++)
    delete it->second;

  Title::g_titles.clear();
  Account::g_accounts.clear();

  delete currGui;

  socketExit();

  close(file);

  gfxExit();

  return 0;
}
