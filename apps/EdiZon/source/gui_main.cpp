#include "gui_main.hpp"
#include "gui_editor.hpp"

#include "save.hpp"
#include "title.hpp"
#include "config_parser.hpp"
#include "account.hpp"

#include "beta_bin.h"

#include "threads.hpp"

#include <string>
#include <sstream>
#include <math.h>

int64_t xOffset, xOffsetNext;
bool finishedDrawing = true;

enum {
  TITLE_SELECT,
  ACCOUNT_SELECT
} selectionState = TITLE_SELECT;

GuiMain::GuiMain() : Gui() {
  m_selected.accountIndex = 0;
  selectionState = TITLE_SELECT;

  if (Title::g_titles.size() != 0)
    xOffset = m_selected.titleIndex > 5 ? m_selected.titleIndex > Title::g_titles.size() - 5 ? 256 * (ceil((Title::g_titles.size() - (Title::g_titles.size() >= 10 ? 11.0F : 9.0F)) / 2.0F) + (Title::g_titles.size() > 10 && Title::g_titles.size() % 2 == 1 ? 1 : 0)) : 256 * ceil((m_selected.titleIndex - 5.0F) / 2.0F) : 0;

  for (auto title : Title::g_titles) {
    if (ConfigParser::hasConfig(title.first) == 0) {
      ConfigParser::g_editableTitles.insert({title.first, true});
    }
  }
}

GuiMain::~GuiMain() {

}

void GuiMain::update() {
  Gui::update();

  if (xOffset != xOffsetNext && finishedDrawing) {
    double deltaOffset = xOffsetNext - xOffset;
    double scrollSpeed = deltaOffset / 40.0F;

    if (xOffsetNext > xOffset)
      xOffset += ceil((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
    else
      xOffset += floor((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
  }
}

void GuiMain::draw() {
  int64_t x = 0, y = 10, currItem = 0;
  int64_t selectedX = 0, selectedY = 0;
  bool tmpEditableOnly = m_editableOnly;

  Gui::beginDraw();

  finishedDrawing = false;

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, 10, COLOR_BLACK);

  if (Title::g_titles.size() == 0) {
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No games or saves found on this system! Please press \uE0EF to exit EdiZon!", ALIGNED_CENTER);
    Gui::endDraw();
    return;
  }

  xOffsetNext = m_selected.titleIndex > 5 ? m_selected.titleIndex > Title::g_titles.size() - 5 ? 256 * (ceil((Title::g_titles.size() - (Title::g_titles.size() >= 10 ? 11.0F : 9.0F)) / 2.0F) + (Title::g_titles.size() > 10 && Title::g_titles.size() % 2 == 1 ? 1 : 0)) : 256 * ceil((m_selected.titleIndex - 5.0F) / 2.0F) : 0;

  m_editableCount = 0;

  for (auto title : Title::g_titles) {
    if (currItem == m_selected.titleIndex) {
      selectedX = x - xOffset;
      selectedY = y;
      m_selected.titleId = title.first;
    }

    if (!tmpEditableOnly || ConfigParser::g_editableTitles.count(title.first)) {
      if (x - xOffset >= -256 && x - xOffset < Gui::g_framebuffer_width) {
        Gui::drawImage(x - xOffset, y, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);

        if (title.first != m_selected.titleId) {
          if (ConfigParser::g_betaTitles[title.first])
            Gui::drawImage(x - xOffset, y, 256, 256, beta_bin, IMAGE_MODE_ABGR32);

          if (y == 266 || title.first == (--Title::g_titles.end())->first)
            Gui::drawShadow(x - xOffset, y, 256, 256);
        }
      }

      y = y == 10 ? 266 : 10;
      x = floor(++currItem / 2.0F) * 256;

      m_editableCount++;
    }
  }

  if (tmpEditableOnly && m_editableCount == 0) {
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No editable games found on this system!", ALIGNED_CENTER);
    Gui::endDraw();
    return;
  }
  else if (selectionState >= TITLE_SELECT && selectedY != 0) {
      Gui::drawRectangled(selectedX - 10, selectedY - 10, 276, 276, selectionState == TITLE_SELECT ? currTheme.highlightColor : currTheme.selectedColor);
      Gui::drawRectangled(selectedX - 5, selectedY - 5, 266, 266, currTheme.selectedButtonColor);
      Gui::drawImage(selectedX, selectedY, 256, 256, Title::g_titles[m_selected.titleId]->getTitleIcon(), IMAGE_MODE_RGB24);

      if (ConfigParser::g_betaTitles[m_selected.titleId])
        Gui::drawImage(selectedX, selectedY, 256, 256, beta_bin, IMAGE_MODE_ABGR32);

      Gui::drawShadow(selectedX - 10, selectedY - 10, 276, 276);
  }

  if (selectionState == TITLE_SELECT) {
    Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);

    if (tmpEditableOnly)
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E4 All titles     \uE0E5 + \uE0E2 Backup all     \uE0E2 Backup     \uE0EF Exit     \uE0F0 Update     \uE0E1 Back     \uE0E0 Ok", ALIGNED_RIGHT);
    else
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E4 Editable titles     \uE0E5 + \uE0E2 Backup all     \uE0E2 Backup     \uE0EF Exit     \uE0F0 Update     \uE0E1 Back     \uE0E0 Ok", ALIGNED_RIGHT);
  }

  if (selectionState >= ACCOUNT_SELECT && Title::g_titles[m_selected.titleId]->getUserIDs().size() > 0) {

      for (u8 i = 0; i < Title::g_titles[m_selected.titleId]->getUserIDs().size(); i++)
        Gui::drawShadow(40 + i * 150, 560, 128, 128);

      Gui::drawRectangled(40 + m_selected.accountIndex * 150 - 10, 550, 148, 148, currTheme.highlightColor);
      Gui::drawRectangled(40 + m_selected.accountIndex * 150 - 5, 555, 138, 138, currTheme.selectedButtonColor);
      Gui::drawShadow(40 + m_selected.accountIndex * 150 - 10, 550, 148, 148);

      u16 accountX = 0;

      for (u128 userID : Title::g_titles[m_selected.titleId]->getUserIDs()) {
        Gui::drawImage(40 + accountX, 560, 128, 128, Account::g_accounts[userID]->getProfileImage(), IMAGE_MODE_RGB24);
        accountX += 150;
      }
  }
  finishedDrawing = true;

  Gui::endDraw();
}

void GuiMain::onInput(u32 kdown) {
  static bool batchClicked = false;

  if (Title::g_titles.size() == 0) return;

  if (kdown & KEY_LEFT) {
    if (selectionState == TITLE_SELECT) {
      if (static_cast<s16>(m_selected.titleIndex - 2) >= 0)
        m_selected.titleIndex -= 2;
    } else if (selectionState == ACCOUNT_SELECT) {
      if (static_cast<s16>(m_selected.accountIndex - 1) >= 0)
        m_selected.accountIndex--;
    }
  } else if (kdown & KEY_RIGHT) {
    if (selectionState == TITLE_SELECT) {
      if (static_cast<u16>(m_selected.titleIndex + 2) < ((!m_editableOnly) ?  Title::g_titles.size() : ConfigParser::g_editableTitles.size()))
        m_selected.titleIndex += 2;
    } else if (selectionState == ACCOUNT_SELECT) {
      if (static_cast<u16>(m_selected.accountIndex + 1) < Title::g_titles[m_selected.titleId]->getUserIDs().size())
        m_selected.accountIndex++;
    }
  } else if (kdown & KEY_UP) {
    if (selectionState == TITLE_SELECT) {
      if ((m_selected.titleIndex % 2) == 1) {
          m_selected.titleIndex--;
      }
    }
  } else if (kdown & KEY_DOWN) {
    if (selectionState == TITLE_SELECT) {
      if ((m_selected.titleIndex % 2) == 0) {
        if (static_cast<u16>(m_selected.titleIndex + 1) < ((!m_editableOnly) ?  Title::g_titles.size() : ConfigParser::g_editableTitles.size()))
          m_selected.titleIndex++;
      }
    }
  }

  if (kdown & KEY_A) {
    if (selectionState == TITLE_SELECT)
      selectionState = ACCOUNT_SELECT;
    else if (selectionState == ACCOUNT_SELECT && Title::g_titles[m_selected.titleId]->getUserIDs().size() > 0) {
      Title::g_currTitle = Title::g_titles[m_selected.titleId];
      Account::g_currAccount = Account::g_accounts[Title::g_titles[m_selected.titleId]->getUserIDs()[m_selected.accountIndex]];
      Gui::g_nextGui = GUI_EDITOR;
    }
  }

  if (selectionState == TITLE_SELECT) {
    if (kdown & KEY_L) {
      m_editableOnly = !m_editableOnly;
      m_selected.titleIndex = 0;
    }

    if (kdown & KEY_X) {
      time_t t = time(nullptr);
      if (batchClicked) {
        bool batchFailed = false;
        (new MessageBox("Are you sure you want to backup all saves\non this console?\nThis might take a while.", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
          if (selection) {
            s16 res;
            u16 failed_titles = 0;
            for (auto title : Title::g_titles) {
              for (u128 userID : Title::g_titles[title.first]->getUserIDs()) {
                if((res = backupSave(title.first, userID, true, t))) {
                  batchFailed = true;
                  failed_titles++;
                }
              }
              if (!batchFailed)
                (new Snackbar("Successfully created backups!"))->show();
              else {
                std::stringstream errorMessage;
                errorMessage << "Failed to backup " << failed_titles << " titles!";
                (new Snackbar(errorMessage.str()))->show();
              }
            }
          }
        })->show();
      }
      else {
        bool batchFailed = false;
        s16 res;

        for (u128 userID : Title::g_titles[m_selected.titleId]->getUserIDs()) {
          if((res = backupSave(m_selected.titleId, userID, true, t))) {
            batchFailed = true;
          }
        }

        if (!batchFailed)
          (new Snackbar("Successfully created backup!"))->show();
        else (new Snackbar("An error occured while creating the backup! Error " + std::to_string(res)))->show();
      }
    }

    if (kdown & KEY_MINUS) {
      (new MessageBox("Checking for updates...", MessageBox::NONE))->show();
      GuiMain::g_shouldUpdate = true;
    }
  }

  batchClicked = (kdown & KEY_R) > 0;

  if (kdown & KEY_B) {
    if (selectionState == ACCOUNT_SELECT) {
        selectionState = TITLE_SELECT;
        m_selected.accountIndex = 0;
    }
  }
}

void GuiMain::onTouch(touchPosition &touch) {
  if (Title::g_titles.size() == 0) return;

  switch (selectionState) {
    case TITLE_SELECT: {
      u8 x = floor((touch.px + xOffset) / 256.0F);
      u8 y = floor(touch.py / 256.0F);
      u8 title = y + x * 2;

      if (y <= 1 && title < Title::g_titles.size()) {
        if (m_editableOnly && title > (m_editableCount - 1)) break;
          if (m_selected.titleIndex == title) {
            Title::g_currTitle = Title::g_titles[m_selected.titleId];
            selectionState = ACCOUNT_SELECT;
          }
          m_selected.titleIndex = title;
        }
      break;
    }
    case ACCOUNT_SELECT: {
      u8 account = floor((touch.px - 40) / 150.0F);

      if (account < Title::g_titles[m_selected.titleId]->getUserIDs().size() && touch.py > 560 && touch.py < (560 + 128)) {
        if (m_selected.accountIndex == account) {
          m_selected.userId = Title::g_titles[m_selected.titleId]->getUserIDs()[account];
          Title::g_currTitle = Title::g_titles[m_selected.titleId];
          Account::g_currAccount = Account::g_accounts[Title::g_titles[m_selected.titleId]->getUserIDs()[m_selected.accountIndex]];
          Gui::g_nextGui = GUI_EDITOR;
        }
        m_selected.accountIndex = account;
      } else {
        selectionState = TITLE_SELECT;
        m_selected.accountIndex = 0;
      }

      break;
    }
  }
}

void GuiMain::onGesture(touchPosition &startPosition, touchPosition &endPosition) {
  if (Title::g_titles.size() == 0) return;

}
