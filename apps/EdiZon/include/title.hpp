#pragma once

#include <switch.h>
#include <string>
#include <map>
#include <memory>

#include "types.h"
#include "save.hpp"
#include "account.hpp"

class Title {
public:
  static inline Title *g_currTitle;
  static inline std::map<u64, Title*> g_titles;

  Title(FsSaveDataInfo& saveInfo);
  ~Title();

  std::string getTitleName();
  std::string getTitleAuthor();
  std::string getTitleVersion();
  u8* getTitleIcon();
  std::vector<u128> getUserIDs();
  void addUserID(u128 userID);
  u64 getTitleID();

private:
  std::vector<u8> m_titleIcon;
  u64 m_titleID;
  std::string m_titleName;
  std::string m_titleAuthor;
  std::string m_titleVersion;
  std::vector<u128> m_userIDs;
  u8 m_errorCode;
};
