#pragma once

#include "gui.hpp"
#include "widget.hpp"
#include "json.hpp"

#include <vector>

#define CONFIG_ROOT "/EdiZon/editor/"

using json = nlohmann::json;

class GuiEditor : public Gui {
public:
  GuiEditor();
  ~GuiEditor();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition &startPosition, touchPosition &endPosition);

  static inline std::vector<u8> g_currSaveFile;
  static inline std::string g_currSaveFileName = "";

private:
  void updateBackupList();
  std::string readMetaDataUsername(std::string path);

  WidgetItems m_widgets;
  std::vector<u8> m_titleIcon;

  std::vector<std::string> m_backupTitles;
  std::vector<std::string> m_backupPaths;
  std::vector<std::string> m_saveFiles;

  color_t m_dominantColor;
  color_t m_textColor;

  s8 m_configFileResult;

  ScriptParser m_scriptParser;

  void updateSaveFileList(std::vector<std::string> saveFilePath, std::string files);
};
