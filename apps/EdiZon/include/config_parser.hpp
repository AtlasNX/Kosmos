#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "widget.hpp"
#include "types.h"
#include "json.hpp"

#define CONFIG_ROOT "/EdiZon/editor/"

using json = nlohmann::json;

class ScriptParser;

class ConfigParser {
public:
    ConfigParser() = delete;

    static s8 hasConfig(u64 titleId);
    static s8 loadConfigFile(u64 titleId, std::string filepath);
    static void unloadConfigFile();
    static void createWidgets(WidgetItems &widgets, ScriptParser &scriptParser);

    static inline std::unordered_map<u64, bool> g_editableTitles;
    static inline std::unordered_map<u64, bool> g_betaTitles;

    template<typename T>
    static inline T getOptionalValue(json j, std::string key, T elseVal) {
        return j.find(key) != j.end() ? j[key].get<T>() : elseVal;
    }

    static inline json& getConfigFile() {
      return m_configFile;
    }

private:
    static inline json m_configFile;
    static inline u8 m_useInsteadTries;

};
