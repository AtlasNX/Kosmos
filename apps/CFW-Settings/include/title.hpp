#pragma once

#include <switch.h>
#include <string>
#include <map>
#include <memory>
#include <vector>

class Title {
public:
  static inline std::map<u64, Title*> g_titles;

  Title(FsSaveDataInfo& saveInfo);
  ~Title();

  std::string getTitleName();
  std::string getTitleAuthor();
  std::string getTitleVersion();
  u64 getTitleID();

  static Result getSaveList(std::vector<FsSaveDataInfo> & saveInfoList);

private:
  u64 m_titleID;
  std::string m_titleName;
  std::string m_titleAuthor;
  std::string m_titleVersion;
  u8 m_errorCode;
};
