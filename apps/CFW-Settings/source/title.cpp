#include "title.hpp"

#include <cstring>

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
  if (R_FAILED(rc) || langentry == nullptr) {
    m_errorCode = 5;
    return;
  }

  m_titleName = std::string(langentry->name);
  m_titleAuthor = std::string(langentry->author);
  m_titleVersion = std::string(buf->nacp.version);

  m_titleID = saveInfo.titleID;

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

u64 Title::getTitleID() {
  return m_titleID;
}

Result Title::getSaveList(std::vector<FsSaveDataInfo> & saveInfoList) {
  Result rc=0;
  FsSaveDataIterator iterator;
  size_t total_entries=0;
  FsSaveDataInfo info;

  rc = fsOpenSaveDataIterator(&iterator, FsSaveDataSpaceId_NandUser);//See libnx fs.h.
  if (R_FAILED(rc)) {
    printf("fsOpenSaveDataIterator() failed: 0x%x\n", rc);
    return rc;
  }

  rc = fsSaveDataIteratorRead(&iterator, &info, 1, &total_entries);//See libnx fs.h.
  if (R_FAILED(rc))
    return rc;
  if (total_entries == 0)
    return MAKERESULT(Module_Libnx, LibnxError_NotFound);

  for (; R_SUCCEEDED(rc) && total_entries > 0;
    rc = fsSaveDataIteratorRead(&iterator, &info, 1, &total_entries)) {
    if (info.SaveDataType == FsSaveDataType_SaveData) {
      saveInfoList.push_back(info);
    }
  }

  fsSaveDataIteratorClose(&iterator);

  return 0;
}
