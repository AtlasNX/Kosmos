#include "gui_editor.hpp"
#include "gui_main.hpp"

#include "title.hpp"
#include "save.hpp"

#include "config_parser.hpp"
#include "script_parser.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <regex>
#include <iterator>

#define COLOR_THRESHOLD 35

GuiEditor::GuiEditor() : Gui() {
  m_titleIcon.reserve(128 * 128 * 3);
  std::vector<u8> smallTitleIcon(32 * 32 * 3);
  std::map<u32, u16> colors;

  m_dominantColor = Gui::makeColor(0xA0, 0xA0, 0xA0, 0xFF);

  Gui::resizeImage(Title::g_currTitle->getTitleIcon(), &m_titleIcon[0], 256, 256, 128, 128);
  Gui::resizeImage(Title::g_currTitle->getTitleIcon(), &smallTitleIcon[0], 256, 256, 32, 32);

  for (u16 i = 0; i < 32 * 32 * 3; i += 3) {
    u32 currColor = smallTitleIcon[i + 0] << 16 | smallTitleIcon[i + 1] << 8 | smallTitleIcon[i + 2];
    colors[currColor]++;
  }

  u32 dominantUseCnt = 0;
  for (auto [color, count] : colors) {
    if (count > dominantUseCnt) {
      color_t colorCandidate = Gui::makeColor((color & 0xFF0000) >> 16, (color & 0x00FF00) >> 8, (color & 0x0000FF), 0xFF);

      if(!(abs(static_cast<s16>(colorCandidate.r) - colorCandidate.g) > COLOR_THRESHOLD || abs(static_cast<s16>(colorCandidate.r) - colorCandidate.b) > COLOR_THRESHOLD))
        continue;

      dominantUseCnt = count;
      m_dominantColor = colorCandidate;
    }
  }

  m_textColor = (m_dominantColor.r > 0x80 && m_dominantColor.g > 0x80 && m_dominantColor.b > 0x80) ? COLOR_BLACK : COLOR_WHITE;

  Widget::g_widgetPage = 0;
  Widget::g_selectedWidgetIndex = 0;
  Widget::g_selectedCategory = "";


  std::stringstream path;
  path << CONFIG_ROOT << std::setfill('0') << std::setw(sizeof(u64) * 2) << std::uppercase << std::hex << Title::g_currTitle->getTitleID() << ".json";

  m_configFileResult = ConfigParser::loadConfigFile(Title::g_currTitle->getTitleID(), path.str());
}

GuiEditor::~GuiEditor() {
  if (GuiEditor::g_currSaveFileName != "") {
    for (auto const& [category, widgets] : m_widgets)
      for(auto widget : widgets)
        delete widget.widget;

    GuiEditor::g_currSaveFile.clear();
  }

  GuiEditor::g_currSaveFile.clear();
  GuiEditor::g_currSaveFileName = "";
  Widget::g_selectedCategory = "";
  m_backupTitles.clear();
  m_backupPaths.clear();

  m_saveFiles.clear();
}

void GuiEditor::update() {
  Gui::update();
}

void GuiEditor::draw() {
  Gui::beginDraw();

  std::stringstream ssTitleId;
  ssTitleId << "0x" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);

  Widget::drawWidgets(this, m_widgets, 150, Widget::g_widgetPage * WIDGETS_PER_PAGE, (Widget::g_widgetPage + 1) * WIDGETS_PER_PAGE);

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, 128, m_dominantColor);
  Gui::drawImage(0, 0, 128, 128, &m_titleIcon[0], IMAGE_MODE_RGB24);
  Gui::drawImage(Gui::g_framebuffer_width - 128, 0, 128, 128, Account::g_currAccount->getProfileImage(), IMAGE_MODE_RGB24);
  Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 128);

  Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), 10, m_textColor, Title::g_currTitle->getTitleName().c_str(), ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), 50, m_textColor, Title::g_currTitle->getTitleAuthor().c_str(), ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), 80, m_textColor, ssTitleId.str().c_str(), ALIGNED_CENTER);

  Gui::drawRectangle(0, Gui::g_framebuffer_height - 73, Gui::g_framebuffer_width, 73, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);

  if (GuiEditor::g_currSaveFileName == "") {
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E2 Backup     \uE0E3 Restore     \uE0E1 Back", ALIGNED_RIGHT);
    switch (m_configFileResult) {
      case 0:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No save file loaded. Press \uE0F0 to select one.", ALIGNED_CENTER);
        break;
      case 1:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "No editor JSON file found. Editing is disabled.", ALIGNED_CENTER);
        break;
      case 2:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "Syntax error in config file! Editing is disabled.", ALIGNED_CENTER);
        break;
      case 3:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "Config file isn't compatible with your game version. Editing is disabled.", ALIGNED_CENTER);
        break;
      case 4:
        Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "Config file loading redirected more than 5 times. Editing is disabled.", ALIGNED_CENTER);
        break;
    }

  } else {
    std::stringstream ssMultiplier;
    ssMultiplier << "\uE074 : x";
    ssMultiplier << Widget::g_stepSizeMultiplier;

    Gui::drawTextAligned(font20, 50, Gui::g_framebuffer_height - 55, currTheme.textColor, ssMultiplier.str().c_str(), ALIGNED_LEFT);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE105 Increase multiplier     \uE0E2 Apply changes     \uE0E1 Cancel     \uE0E0 Ok", ALIGNED_RIGHT);
  }

  if (m_widgets[Widget::g_selectedCategory].size() > WIDGETS_PER_PAGE) {
    for (u8 page = 0; page < Widget::g_widgetPageCnt[Widget::g_selectedCategory]; page++) {
      Gui::drawRectangle((Gui::g_framebuffer_width / 2) - Widget::g_widgetPageCnt[Widget::g_selectedCategory] * 15 + page * 30 , 608, 20, 20, currTheme.separatorColor);
      if (page == Widget::g_widgetPage)
        Gui::drawRectangled((Gui::g_framebuffer_width / 2) - Widget::g_widgetPageCnt[Widget::g_selectedCategory] * 15 + page * 30 + 4, 612, 12, 12, currTheme.highlightColor);
    }

    Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2) - Widget::g_widgetPageCnt[Widget::g_selectedCategory] * 15 - 30, 602, currTheme.textColor, "\uE0A4", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2) + Widget::g_widgetPageCnt[Widget::g_selectedCategory] * 15 + 23, 602, currTheme.textColor, "\uE0A5", ALIGNED_CENTER);

  }

  Gui::endDraw();
}

void GuiEditor::updateBackupList() {
  DIR *dir_batch;
  DIR *dir_users;
  DIR *dir_titles;

  struct dirent *ent_timestamp;
  struct dirent *ent_user;

  std::map<std::string, std::string> backups;
  std::string metadataUsername;

  m_backupTitles.clear();
  m_backupPaths.clear();

  std::stringstream path;

  //Read root saves
  path << "/EdiZon/" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();
  if ((dir_titles = opendir(path.str().c_str())) != nullptr) {
    while ((ent_timestamp = readdir(dir_titles)) != nullptr) {
      metadataUsername = GuiEditor::readMetaDataUsername(path.str() + "/" + std::string(ent_timestamp->d_name) + "/edizon_save_metadata.json");
      if (metadataUsername.empty())
        metadataUsername = "By an unknown user";
      else
        metadataUsername = "By " + metadataUsername;

      backups.insert(std::make_pair(std::string(ent_timestamp->d_name) +  ", " + metadataUsername, path.str() + "/" + std::string(ent_timestamp->d_name)));
    }
    closedir(dir_titles);
  }

  //Read batch saves
  if ((dir_batch = opendir("/EdiZon/batch")) != nullptr) {
    while ((ent_timestamp = readdir(dir_batch)) != nullptr) {
      path.str("");
      path << "/EdiZon/batch/" << std::string(ent_timestamp->d_name);
      if ((dir_users = opendir(path.str().c_str())) != nullptr) {
        while ((ent_user = readdir(dir_users)) != nullptr) {
          path.str("");
          path << "/EdiZon/batch/" << std::string(ent_timestamp->d_name) << "/" << std::string(ent_user->d_name) << "/" << std::setfill('0') << std::setw(16) << std::uppercase << std::hex << Title::g_currTitle->getTitleID();
          if ((dir_titles = opendir(path.str().c_str())) != nullptr) {
            metadataUsername = GuiEditor::readMetaDataUsername(path.str() + "/edizon_save_metadata.json");

            if (metadataUsername.empty())
              metadataUsername = "By an unknown user [B]";
            else
              metadataUsername = "By " + metadataUsername + " [B]";

            backups.insert(std::make_pair(std::string(ent_timestamp->d_name) +  ", " + metadataUsername, path.str()));

            closedir(dir_titles);
          }
        }
        closedir(dir_users);
      }
    }
    closedir(dir_batch);
  }

  for (auto [title, path] : backups) {
    m_backupTitles.push_back(title);
    m_backupPaths.push_back(path);
  }

  std::reverse(m_backupTitles.begin(), m_backupTitles.end());
  std::reverse(m_backupPaths.begin(), m_backupPaths.end());
}

std::string GuiEditor::readMetaDataUsername(std::string path) {
  json metadata_json;

  std::ifstream metadata_file (path);
  if (metadata_file.is_open())
  {
    metadata_file >> metadata_json;
    metadata_file.close();
    try {
      return metadata_json["user_name"].get<std::string>();
    } catch (json::parse_error& e) {
	  }
  }
  else
  {
    printf("Unable to open metadata file\n");
  }
  return "";
}

void GuiEditor::updateSaveFileList(std::vector<std::string> saveFilePath, std::string files) {
  DIR *dir;
  struct dirent *ent;
  FsFileSystem fs;

  std::vector<std::string> pathsOld;
  std::vector<std::string> paths;

  if (mountSaveByTitleAccountIDs(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), fs))
    return;

  if (saveFilePath[0] != "") {
    dir = opendir("save:/");

    while ((ent = readdir(dir)) != nullptr) {
      if (std::regex_match(std::string(ent->d_name), std::regex(saveFilePath[0]))) {
        pathsOld.push_back(std::string(ent->d_name) + "/");
      }
    }

    closedir(dir);

    for (u16 i = 1; i < saveFilePath.size(); i++) {
      for (auto path : pathsOld) {
        dir = opendir(path.c_str());
        while ((ent = readdir(dir)) != nullptr) {
          if (std::regex_match(std::string(ent->d_name), std::regex(saveFilePath[i]))) {
            std::string newPath = path;
            newPath += "/";
            newPath += ent->d_name;
            paths.push_back(newPath);
          }
        }

        closedir(dir);
      }

      pathsOld = paths;
      paths.clear();
    }
  } else pathsOld.push_back("");

  for (auto path : pathsOld) {
    std::string finalSaveFilePath = std::string("save:/") + path;
    if ((dir = opendir(finalSaveFilePath.c_str())) != nullptr) {
      std::regex validSaveFileNames(files);

      while ((ent = readdir(dir)) != nullptr) {
        if (std::regex_match(ent->d_name, validSaveFileNames))
          m_saveFiles.push_back(path + ent->d_name);
      }
      closedir(dir);
    }
  }

  std::reverse(m_saveFiles.begin(), m_saveFiles.end());

  fsdevUnmountDevice(SAVE_DEV);
  fsFsClose(&fs);

}

void GuiEditor::onInput(u32 kdown) {
if (GuiEditor::g_currSaveFileName == "") { /* No savefile loaded */

  if (kdown & KEY_MINUS) {
    if (m_configFileResult != 0) return;
    m_saveFiles.clear();

    updateSaveFileList(ConfigParser::getConfigFile()["saveFilePaths"], ConfigParser::getConfigFile()["files"]);

    (new ListSelector("Edit save file", "\uE0E0  Select      \uE0E1  Back", m_saveFiles))->setInputAction([&](u32 k, u16 selectedItem){
      if (k & KEY_A) {
        if (m_saveFiles.size() != 0) {
          size_t length;
          Widget::g_selectedWidgetIndex = 0;
          Widget::g_selectedCategory = "";
          Widget::g_selectedRow = CATEGORIES;
          Widget::g_categoryYOffset = 0;
          GuiEditor::g_currSaveFileName = m_saveFiles[Gui::g_currListSelector->selectedItem].c_str();

          if (loadSaveFile(&GuiEditor::g_currSaveFile, &length, Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), GuiEditor::g_currSaveFileName.c_str()) == 0) {
              m_scriptParser.setLuaSaveFileBuffer(&g_currSaveFile[0], length, ConfigParser::getOptionalValue<std::string>(ConfigParser::getConfigFile(), "encoding", "ascii"));
              ConfigParser::createWidgets(m_widgets, m_scriptParser);
              m_scriptParser.luaInit(ConfigParser::getConfigFile()["filetype"]);
              if (ConfigParser::g_betaTitles[Title::g_currTitle->getTitleID()])
                (new MessageBox("Please create a backup before using this beta config.", MessageBox::OKAY))->show();
            }
            else {
              (new Snackbar("Failed to load save file! Is it empty?"))->show();
              GuiEditor::g_currSaveFile.clear();
              GuiEditor::g_currSaveFileName = "";

              for (auto const& [category, widgets] : m_widgets)
                for(auto widget : widgets)
                  delete widget.widget;

              m_widgets.clear();
            }
            Gui::Gui::g_currListSelector->hide();
          }
        }
      })->show();
    }

    if (kdown & KEY_B) {
      Gui::g_nextGui = GUI_MAIN;
    }

    if (kdown & KEY_X) {
      s16 res;

      if(!(res = backupSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID())))
        (new Snackbar("Successfully created backup!"))->show();
      else (new Snackbar("An error occured while creating the backup! Error " + std::to_string(res)))->show();
    }

    if (kdown & KEY_Y) {
      updateBackupList();

      (new ListSelector("Restore Backup", "\uE0E0  Restore     \uE0E2  Delete      \uE0E1  Back", m_backupTitles))->setInputAction([&](u32 k, u16 selectedItem){
        if (k & KEY_A) {
          if (m_backupTitles.size() != 0) {
              (new MessageBox("Are you sure you want to inject this backup?", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
                if (selection) {
                  s16 res;

                  if(!(res = restoreSave(Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), m_backupPaths[Gui::Gui::g_currListSelector->selectedItem].c_str())))
                    (new Snackbar("Successfully restored backup!"))->show();
                  else (new Snackbar("An error occured while restoring the backup! Error " + std::to_string(res)))->show();

                  Gui::Gui::g_currListSelector->hide();
                }
              })->show();
          }
        }

        if (k & KEY_X) {
          std::stringstream path;
          deleteDirRecursively(m_backupPaths[Gui::Gui::g_currListSelector->selectedItem].c_str(), false);
          updateBackupList();

          if (Gui::Gui::g_currListSelector->selectedItem == m_backupTitles.size() && Gui::Gui::g_currListSelector->selectedItem > 0)
            Gui::Gui::g_currListSelector->selectedItem--;
        }
      })->show();
    }

    if (kdown & KEY_ZL) {
      Title *nextTitle = nullptr;
      bool isCurrTitle = false;

      for (auto title : Title::g_titles) {
        if (isCurrTitle) {
          nextTitle = title.second;
          break;
        }

        isCurrTitle = title.second == Title::g_currTitle;
      }

      if (nextTitle == nullptr)
        nextTitle = Title::g_titles.begin()->second;

      Title::g_currTitle = nextTitle;
      Account::g_currAccount = Account::g_accounts[Title::g_currTitle->getUserIDs()[0]];
      Gui::g_nextGui = GUI_EDITOR;
    }

    if (kdown & KEY_ZR) {
      Account *nextAccount = nullptr;
      bool isCurrAccount = false;

      for (auto userID : Title::g_currTitle->getUserIDs()) {
        if (isCurrAccount) {
          nextAccount = Account::g_accounts[userID];
          break;
        }
        isCurrAccount = userID == Account::g_currAccount->getUserID();
      }

      if (nextAccount == nullptr)
        nextAccount = Account::g_accounts[Title::g_currTitle->getUserIDs()[0]];

      if (Title::g_currTitle->getUserIDs().size() != 1) {
        Account::g_currAccount = nextAccount;
        Gui::g_nextGui = GUI_EDITOR;
      } else nextAccount = nullptr;
    }
  } /* Savefile loaded */
  else {
    if (Widget::g_selectedRow == WIDGETS) { /* Widgets row */
      if (kdown & KEY_L) {
        if (Widget::g_widgetPage > 0)
          Widget::g_widgetPage--;
        Widget::g_selectedWidgetIndex = WIDGETS_PER_PAGE * Widget::g_widgetPage;
      }

      if (kdown & KEY_R) {
        if (Widget::g_widgetPage < Widget::g_widgetPageCnt[Widget::g_selectedCategory] - 1)
          Widget::g_widgetPage++;
        Widget::g_selectedWidgetIndex = WIDGETS_PER_PAGE * Widget::g_widgetPage ;
      }

      if (kdown & KEY_B) {
        Widget::g_selectedRow = CATEGORIES;
        Widget::g_selectedWidgetIndex = std::distance(Widget::g_categories.begin(), std::find(Widget::g_categories.begin(), Widget::g_categories.end(), Widget::g_selectedCategory));
      }

      if (kdown & KEY_UP) {
        if (Widget::g_selectedWidgetIndex > 0)
          Widget::g_selectedWidgetIndex--;
        Widget::g_widgetPage = floor(Widget::g_selectedWidgetIndex / WIDGETS_PER_PAGE);
      }

      if (kdown & KEY_DOWN) {
        if (Widget::g_selectedWidgetIndex < m_widgets[Widget::g_selectedCategory].size() - 1)
          Widget::g_selectedWidgetIndex++;
        Widget::g_widgetPage = floor(Widget::g_selectedWidgetIndex / WIDGETS_PER_PAGE);
      }

    } else { /* Categories row */
      if (kdown & KEY_B) {
        (new MessageBox("Are you sure you want to discard your changes?", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
          if (selection) {
            m_scriptParser.luaDeinit();

            GuiEditor::g_currSaveFile.clear();
            GuiEditor::g_currSaveFileName = "";

            for (auto const& [category, widgets] : m_widgets)
              for(auto widget : widgets)
                delete widget.widget;

            m_widgets.clear();
            Widget::g_categories.clear();
          }
        })->show();

        return;
      }

      if (kdown & KEY_UP) {
        if (Widget::g_selectedWidgetIndex > 0) {
          Widget::g_selectedWidgetIndex--;

          if (Widget::g_selectedWidgetIndex < Widget::g_categories.size() - 7 && Widget::g_categoryYOffset != 0)
            Widget::g_categoryYOffset--;
        }
        Widget::g_selectedCategory = Widget::g_categories[Widget::g_selectedWidgetIndex];
        Widget::g_widgetPage = 0;
      }

      if (kdown & KEY_DOWN) {
        if (Widget::g_selectedWidgetIndex < Widget::g_categories.size() - 1) {
          Widget::g_selectedWidgetIndex++;

          if (Widget::g_selectedWidgetIndex > 6 && Widget::g_categoryYOffset < Widget::g_categories.size() - 8)
            Widget::g_categoryYOffset++;
        }
        Widget::g_selectedCategory = Widget::g_categories[Widget::g_selectedWidgetIndex];
        Widget::g_widgetPage = 0;
      }
    }
    /* Categories and widgets row */
    if (kdown & KEY_X) {
      (new MessageBox("Are you sure you want to edit these values?", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
        if (selection) {
          std::vector<u8> buffer;

          m_scriptParser.getModifiedSaveFile(buffer);

          if(!storeSaveFile(&buffer[0], buffer.size(), Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), GuiEditor::g_currSaveFileName.c_str()))
            (new Snackbar("Successfully injected modified values!"))->show();
          else
            (new Snackbar("Injection of modified values failed!"))->show();

          GuiEditor::g_currSaveFile.clear();
          GuiEditor::g_currSaveFileName = "";
          Widget::g_widgetPage = 0;

          for (auto const& [category, widgets] : m_widgets)
            for(auto widget : widgets)
              delete widget.widget;

          Widget::g_categories.clear();
          m_widgets.clear();
        }
      })->show();

      return;
    }

    Widget::handleInput(kdown, m_widgets);
  }
}

void GuiEditor::onTouch(touchPosition &touch) {
  //s8 widgetTouchPos = floor((touch.py - 150) / (static_cast<float>(WIDGET_HEIGHT) + WIDGET_SEPARATOR)) + WIDGETS_PER_PAGE * Widget::g_widgetPage;

  if (GuiEditor::g_currSaveFileName == "") {
    if (touch.px < 128 && touch.py < 128) {
      Title *nextTitle = nullptr;
      bool isCurrTitle = false;

      for (auto title : Title::g_titles) {
        if (isCurrTitle) {
          nextTitle = title.second;
          break;
        }

        isCurrTitle = title.second == Title::g_currTitle;
      }

      if (nextTitle == nullptr)
        nextTitle = Title::g_titles.begin()->second;

      Title::g_currTitle = nextTitle;
      Account::g_currAccount = Account::g_accounts[Title::g_currTitle->getUserIDs()[0]];
      Gui::g_nextGui = GUI_EDITOR;
    }

    if (touch.px > Gui::g_framebuffer_width - 128 && touch.py < 128) {
      Account *nextAccount = nullptr;
      bool isCurrAccount = false;

      for (auto userID : Title::g_currTitle->getUserIDs()) {
        if (isCurrAccount) {
          nextAccount = Account::g_accounts[userID];
          break;
        }
        isCurrAccount = userID == Account::g_currAccount->getUserID();
      }

      if (nextAccount == nullptr)
        nextAccount = Account::g_accounts[Title::g_currTitle->getUserIDs()[0]];

      if (Title::g_currTitle->getUserIDs().size() != 1) {
        Account::g_currAccount = nextAccount;
        Gui::g_nextGui = GUI_EDITOR;
      } else nextAccount = nullptr;
    }
  }
}

void GuiEditor::onGesture(touchPosition &startPosition, touchPosition &endPosition) {

}
