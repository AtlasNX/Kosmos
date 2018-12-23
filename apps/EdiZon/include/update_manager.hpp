#pragma once

#include <switch.h>
#include <string>
#include <vector>

typedef enum Updates { NONE, ERROR, EDITOR, EDIZON } Updates;

class UpdateManager {
public:
  UpdateManager();
  ~UpdateManager();

  Updates checkUpdate();

private:
  std::vector<std::pair<std::string, std::string>> m_downloadPaths;
  std::string m_versionString;

};
