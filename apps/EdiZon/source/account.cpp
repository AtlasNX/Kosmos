#include "account.hpp"

#include "gui.hpp"

#include <cstring>
#include <memory>

extern "C" {
  #include "nanojpeg.h"
}

Account::Account(u128 userID) : m_userID(userID) {
  accountInitialize();

  accountGetProfile(&m_profile, userID);
  accountProfileGet(&m_profile, &m_userData, &m_profileBase);
  accountProfileGetImageSize(&m_profile, &m_profileImageSize);

  m_userName = std::string(m_profileBase.username);

  std::vector<u8> buffer(m_profileImageSize);
  u8 *decodedBuffer;
  size_t imageSize = 0;

  accountProfileLoadImage(&m_profile, &buffer[0], m_profileImageSize, &imageSize);
  njInit();
  njDecode(&buffer[0], imageSize);

  m_profileImage.reserve(128 * 128 * 3);
  decodedBuffer = njGetImage();
  Gui::resizeImage(decodedBuffer, &m_profileImage[0], 256, 256, 128, 128);

  njDone();

  accountProfileClose(&m_profile);
  accountExit();
}

Account::~Account() {
}

u128 Account::getUserID() {
  return m_userID;
}

std::string Account::getUserName() {
  return m_userName;
}

u8* Account::getProfileImage() {
  return &m_profileImage[0];
}
