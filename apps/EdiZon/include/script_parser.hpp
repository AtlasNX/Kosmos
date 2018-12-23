#pragma once

#define LLONG_MAX INT64_MAX
#define LLONG_MIN INT64_MIN
#include "lua.hpp"

#include <string>
#include <vector>

#include <switch.h>

class ScriptParser {
public:
  ScriptParser();
  ~ScriptParser();

  void luaInit(std::string filetype);
  void luaDeinit();

  s64 getValueFromSaveFile();
  std::string getStringFromSaveFile();
  void setValueInSaveFile(s64 value);
  void setStringInSaveFile(std::string value);
  void getModifiedSaveFile(std::vector<u8> &buffer);

  void setLuaArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs);
  void setLuaSaveFileBuffer(u8 *buffer, size_t bufferSize, std::string encoding);

  int lua_getSaveFileBuffer(lua_State *state);
  int lua_getSaveFileString(lua_State *state);
  int lua_getStrArgs(lua_State *state);
  int lua_getIntArgs(lua_State *state);

  static double evaluateEquation(std::string equation, s64 value);

private:
  std::string m_filetype;
  std::vector<u8> m_buffer;
  u64 m_bufferSize;

  lua_State *m_luaState;
  std::vector<s32> m_intArgs;
  std::vector<std::string> m_strArgs;

  enum {
    ASCII,
    UTF_8,
    UTF_16LE,
    UTF_16BE
  } m_encoding = ASCII;

  static void printError(lua_State *luaState);
};
