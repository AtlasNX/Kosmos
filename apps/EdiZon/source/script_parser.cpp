#include "script_parser.hpp"

#include <iostream>
#include <algorithm>

#include "encoding.hpp"

int lua_getSaveFileBuffer(lua_State *state);
int lua_getStrArguments(lua_State *state);
int lua_getIntArguments(lua_State *state);

typedef int (ScriptParser::*mem_func)(lua_State *s);

template <mem_func func>
int dispatch(lua_State *s) {
  ScriptParser * ptr = *static_cast<ScriptParser**>(lua_getextraspace(s));
  return ((*ptr).*func)(s);
}

ScriptParser::ScriptParser() {
  m_luaState = nullptr;
  m_buffer.clear();
}

ScriptParser::~ScriptParser() {
  if (m_luaState != nullptr) {
    lua_close(m_luaState);
    m_luaState = nullptr;
  }

}

void ScriptParser::printError(lua_State *luaState) {
  printf("%s\n", lua_tostring(luaState, -1));
}

void ScriptParser::luaInit(std::string filetype) {
  m_luaState = luaL_newstate();

  luaL_openlibs(m_luaState);

  *static_cast<ScriptParser**>(lua_getextraspace(m_luaState)) = this;

  const luaL_Reg regs[] {
    { "getSaveFileBuffer", &dispatch<&ScriptParser::lua_getSaveFileBuffer> },
    { "getSaveFileString", &dispatch<&ScriptParser::lua_getSaveFileString> },
    { "getStrArgs", &dispatch<&ScriptParser::lua_getStrArgs> },
    { "getIntArgs", &dispatch<&ScriptParser::lua_getIntArgs> },
    { nullptr, nullptr }
  };

  luaL_newlib(m_luaState, regs);
  lua_setglobal(m_luaState, "edizon");

  std::string path = "/EdiZon/editor/scripts/";
  path += filetype;
  path += ".lua";

  luaL_loadfile(m_luaState, path.c_str());

  if(lua_pcall(m_luaState, 0, 0, 0))
    printError(m_luaState);

  printf("Lua interpreter initialized!\n");
}

void ScriptParser::luaDeinit() {
  if (m_luaState != nullptr) {
    lua_close(m_luaState);
    m_luaState = nullptr;
  }
}

void ScriptParser::setLuaSaveFileBuffer(u8 *buffer, size_t bufferSize, std::string encoding) {
  std::vector<u8> utf8;

  std::transform(encoding.begin(), encoding.end(), encoding.begin(), ::tolower);

  if (encoding == "ascii")
    m_encoding = ASCII;
  else if (encoding == "utf-8")
    m_encoding = UTF_8;
  else if (encoding == "utf-16le")
    m_encoding = UTF_16LE;
  else if (encoding == "utf-16be")
    m_encoding = UTF_16BE;
  else printf("Lua init warning: Invalid encoding, using ASCII\n");

  switch (m_encoding) {
    case UTF_16BE:
      utf8 = Encoding::uft16beToUtf8(buffer, bufferSize);
      break;
    case UTF_16LE:
      utf8 = Encoding::uft16leToUtf8(buffer, bufferSize);
      break;
    case ASCII: [[fallthrough]]
    case UTF_8: [[fallthrough]]
    default:
      utf8 = std::vector<u8>(buffer, buffer + bufferSize);
      break;
  }

  m_buffer.clear();

  m_bufferSize = utf8.size();
  m_buffer.reserve(m_bufferSize);

  for (u32 i = 0; i < this->m_bufferSize; i++)
    m_buffer[i] = utf8[i];
}

void ScriptParser::setLuaArgs(std::vector<s32> intArgs, std::vector<std::string> strArgs) {
  m_intArgs = intArgs;
  m_strArgs = strArgs;
}

s64 ScriptParser::getValueFromSaveFile() {
  s64 out;

  lua_getglobal(m_luaState, "getValueFromSaveFile");
  if(lua_pcall(m_luaState, 0, 1, 0))
    printError(m_luaState);

  out = lua_tointeger(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}


std::string ScriptParser::getStringFromSaveFile() {
  std::string out;

  lua_getglobal(m_luaState, "getStringFromSaveFile");
  if (lua_pcall(m_luaState, 0, 1, 0))
    printError(m_luaState);

  out = lua_tostring(m_luaState, -1);
  lua_pop(m_luaState, 1);

  return out;
}

void ScriptParser::setValueInSaveFile(s64 value) {
  lua_getglobal(m_luaState, "setValueInSaveFile");
  lua_pushinteger(m_luaState, value);
  if (lua_pcall(m_luaState, 1, 0, 0))
    printError(m_luaState);
}

void ScriptParser::setStringInSaveFile(std::string value) {
  lua_getglobal(m_luaState, "setStringInSaveFile");
  lua_pushstring(m_luaState, value.c_str());
  if (lua_pcall(m_luaState, 1, 0, 0))
    printError(m_luaState);
}

void ScriptParser::getModifiedSaveFile(std::vector<u8> &buffer) {
  std::vector<u8> encoded;

  lua_getglobal(m_luaState, "getModifiedSaveFile");
  if (lua_pcall(m_luaState, 0, 1, 0))
    printError(m_luaState);

  lua_pushnil(m_luaState);

    while (lua_next(m_luaState, 1)) {
        encoded.push_back(lua_tointeger(m_luaState, -1));
        lua_pop(m_luaState, 1);
    }

  lua_pop(m_luaState, 1);

  switch (m_encoding) {
    case UTF_16BE:
      buffer = Encoding::utf8ToUtf16be(&encoded[0], encoded.size());
      break;
    case UTF_16LE:
      buffer = Encoding::utf8ToUtf16le(&encoded[0], encoded.size());
      break;
    case ASCII: [[fallthrough]]
    case UTF_8: [[fallthrough]]
    default:
      buffer = encoded;
      break;
  }
}

int ScriptParser::lua_getSaveFileBuffer(lua_State *state) {
  lua_createtable(state, m_bufferSize, 0);

  for (u64 i = 0; i < m_bufferSize; i++) {
    lua_pushinteger(state, i + 1);
    lua_pushinteger(state, m_buffer[i]);
    lua_settable(state, -3);
  }

  return 1;
}

int ScriptParser::lua_getSaveFileString(lua_State *state) {
  std::string str = reinterpret_cast<char*>(&m_buffer[0]);

  str += '\x00';

  str[m_bufferSize] = 0x00;

  lua_pushstring(state, str.c_str());

  return 1;
}

int ScriptParser::lua_getStrArgs(lua_State *state) {
  lua_createtable(state, m_strArgs.size(), 0);

  for (u64 i = 0; i < m_strArgs.size(); i++) {
    lua_pushinteger(state, i + 1);
    lua_pushstring(state, m_strArgs[i].c_str());
    lua_settable(state, -3);
  }

  return 1;
}

int ScriptParser::lua_getIntArgs(lua_State *state) {
  lua_createtable(state, m_intArgs.size(), 0);

  for (u64 i = 0; i < m_intArgs.size(); i++) {
    lua_pushinteger(state, i + 1);
    lua_pushinteger(state, m_intArgs[i]);
    lua_settable(state, -3);
  }

  return 1;
}

double ScriptParser::evaluateEquation(std::string equation, s64 value) {
  lua_State *s = luaL_newstate();
  double ret;
  std::string func = "function eval(value)\n";
  func += "return ";
  func += equation;
  func += "\nend";

  luaL_dostring(s, func.c_str());
  lua_getglobal(s, "eval");
  lua_pushinteger(s, value);
  if (lua_pcall(s, 1, 1, 0))
    printError(s);
  ret = lua_tonumber(s, -1);

  if (s != nullptr) {
    lua_close(s);
    s = nullptr;
  }

  return ret;
}
