#include "title.hpp"

extern "C" {
#include "nanojpeg.h"
}

Title::Title(FsSaveDataInfo& saveInfo) {
  Result rc=0;

  std::unique_ptr<NsApplicationControlData> buf = std::make_unique<NsApplicationControlData>();
  size_t outsize=0;

  NacpLanguageEntry *langentry = nullptr;

  if (buf == nullptr) {
    m_errorCode = 1;
    return;
  }
  memset(buf.get(), 0, sizeof(NsApplicationControlData));

  rc = nsInitialize();
  if (R_FAILED(rc)) {
    m_errorCode = 2;
    return;
  }

  rc = nsGetApplicationControlData(1, saveInfo.titleID, buf.get(), sizeof(NsApplicationControlData), &outsize);
  if (R_FAILED(rc)) {
    m_errorCode = 3;
    return;
  }

  if (outsize < sizeof(buf->nacp)) {
    m_errorCode = 4;
    return;
  }

  rc = nacpGetLanguageEntry(&buf->nacp, &langentry);
  if (R_FAILED(rc) || langentry==nullptr) {
    m_errorCode = 5;
    return;
  }

  m_titleName = std::string(langentry->name);
  m_titleAuthor = std::string(langentry->author);
  m_titleVersion = std::string(buf->nacp.version);

  m_titleID = saveInfo.titleID;

  njInit();

  size_t iconbytesize = outsize-sizeof(buf->nacp);
  size_t imagesize = 256 * 256 * 3;

  if (njDecode(buf->icon, iconbytesize) != NJ_OK) {
    m_errorCode = 6;
    njDone();
    return;
  }

  if (njGetWidth() != 256 || njGetHeight() != 256 || (size_t)njGetImageSize() != imagesize || njIsColor() != 1) {
    m_errorCode = 7;
    njDone();
    return;
  }

  u8* ptr = nullptr;

  ptr = njGetImage();
  if (ptr == nullptr) {
    m_errorCode = 8;
    njDone();
    return;
  }

  m_titleIcon.reserve(imagesize);
  memcpy(&m_titleIcon[0], ptr, imagesize);
  ptr = nullptr;

  njDone();
  nsExit();
}

Title::~Title() {
}

std::string Title::getTitleName() {
  return m_titleName;
}

std::string Title::getTitleAuthor() {
  return m_titleAuthor;
}

std::string Title::getTitleVersion() {
  return m_titleVersion;
}

u8* Title::getTitleIcon() {
  return &m_titleIcon[0];
}

u64 Title::getTitleID() {
  return m_titleID;
}

std::vector<u128> Title::getUserIDs() {
  return m_userIDs;
}

void Title::addUserID(u128 userID) {
  m_userIDs.push_back(userID);
}
