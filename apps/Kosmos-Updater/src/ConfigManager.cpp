// Kosmos Updater
// Copyright (C) 2018 Steven Mattera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "ConfigManager.hpp"

void ConfigManager::initialize() {
    config_init(&_cfg);
    config_init(&_internalDb);

    if(!config_read_file(&_cfg, CONFIG_FILENAME.c_str())) {
        config_setting_t * root, * setting;
        root = config_root_setting(&_cfg);

        setting = config_setting_add(root, HOST_KEY.c_str(), CONFIG_TYPE_STRING);
        config_setting_set_string(setting, HOST_DEF.c_str());

        setting = config_setting_add(root, CHANNEL_KEY.c_str(), CONFIG_TYPE_STRING);
        config_setting_set_string(setting, CHANNEL_DEF.c_str());

        setting = config_setting_add(root, BUNDLE_KEY.c_str(), CONFIG_TYPE_STRING);
        config_setting_set_string(setting, BUNDLE_DEF.c_str());

        setting = config_setting_add(root, VERSION_KEY.c_str(), CONFIG_TYPE_STRING);
        config_setting_set_string(setting, VERSION_DEF.c_str());

        setting = config_setting_add(root, IGNORE_KEY.c_str(), CONFIG_TYPE_ARRAY);

        setting = config_setting_add(root, AUTOUPDATE_KEY.c_str(), CONFIG_TYPE_BOOL);
        config_setting_set_bool(setting, AUTOUPDATE_DEF);

        setting = config_setting_add(root, PROXY_ENABLED_KEY.c_str(), CONFIG_TYPE_BOOL);
        config_setting_set_bool(setting, PROXY_ENABLED_DEF);
        
        setting = config_setting_add(root, PROXY_URL_KEY.c_str(), CONFIG_TYPE_STRING);
        config_setting_set_string(setting, PROXY_URL_DEF.c_str());

        setting = config_setting_add(root, PROXY_USERNAME_KEY.c_str(), CONFIG_TYPE_STRING);
        config_setting_set_string(setting, PROXY_USERNAME_DEF.c_str());

        setting = config_setting_add(root, PROXY_PASSWORD_KEY.c_str(), CONFIG_TYPE_STRING);
        config_setting_set_string(setting, PROXY_PASSWORD_DEF.c_str());

        config_write_file(&_cfg, CONFIG_FILENAME.c_str());
    }

    if(!config_read_file(&_internalDb, INTERNAL_FILENAME.c_str())) {
        config_setting_t * root, * setting;
        root = config_root_setting(&_internalDb);

        setting = config_setting_add(root, INSTALLED_FILES_KEY.c_str(), CONFIG_TYPE_ARRAY);
        
        setting = config_setting_add(root, DISABLED_GAME_CART_KEY.c_str(), CONFIG_TYPE_BOOL);
        config_setting_set_bool(setting, DISABLED_GAME_CART_DEF);
        
        setting = config_setting_add(root, RECEIVED_EXFAT_WARNING_KEY.c_str(), CONFIG_TYPE_BOOL);
        config_setting_set_bool(setting, RECEIVED_EXFAT_WARNING_DEF);

        config_write_file(&_internalDb, INTERNAL_FILENAME.c_str());
    }
}

void ConfigManager::dealloc() {
    config_destroy(&_cfg);
    config_destroy(&_internalDb);
}

string ConfigManager::getHost() {
    string host = _readString(HOST_KEY, HOST_DEF, _cfg);

    if (host == "http://sdfu.stevenmattera.com") {
        _writeString(HOST_KEY, HOST_DEF, _cfg, CONFIG_FILENAME);
        return HOST_DEF;
    }

    return host;
}

string ConfigManager::getChannel() {
    string channel = _readString(CHANNEL_KEY, CHANNEL_DEF, _cfg);

    if (channel != "stable" && channel != "bleeding-edge") {
        setChannel(CHANNEL_DEF);
        return CHANNEL_DEF;
    }

    return channel;
}

string ConfigManager::getBundle() {
    string bundle = _readString(BUNDLE_KEY, BUNDLE_DEF, _cfg);

    if (bundle != "kosmos" && bundle != "atmosphere") {
        setBundle(BUNDLE_DEF);
        return BUNDLE_DEF;
    }

    return bundle;
}

string ConfigManager::getCurrentVersion() {
    return _readString(VERSION_KEY, VERSION_DEF, _cfg);
}

vector<string> ConfigManager::getFilesToIgnore() {
    vector<string> defaultValue;
    return _readArrayOfStrings(IGNORE_KEY, defaultValue, _cfg);
}

bool ConfigManager::shouldAutoUpdate() {
    #ifdef DEBUG
        return false;
    #endif

    return _readBoolean(AUTOUPDATE_KEY, AUTOUPDATE_DEF, _cfg);
}

bool ConfigManager::shouldUseProxy() {
    return _readBoolean(PROXY_ENABLED_KEY, PROXY_ENABLED_DEF, _cfg);
}

string ConfigManager::getProxy() {
    return _readString(PROXY_URL_KEY, PROXY_URL_DEF, _cfg);
}

string ConfigManager::getProxyUsername() {
    return _readString(PROXY_USERNAME_KEY, PROXY_USERNAME_DEF, _cfg);
}

string ConfigManager::getProxyPassword() {
    return _readString(PROXY_PASSWORD_KEY, PROXY_PASSWORD_DEF, _cfg);
}

bool ConfigManager::setChannel(string channel) {
    return _writeString(CHANNEL_KEY, channel, _cfg, CONFIG_FILENAME);
}

bool ConfigManager::setBundle(string bundle) {
    return _writeString(BUNDLE_KEY, bundle, _cfg, CONFIG_FILENAME);
}

bool ConfigManager::setCurrentVersion(string version) {
    return _writeString(VERSION_KEY, version, _cfg, CONFIG_FILENAME);
}

vector<string> ConfigManager::getInstalledFiles() {
    vector<string> defaultValue;
    return _readArrayOfStrings(INSTALLED_FILES_KEY, defaultValue, _internalDb);
}

bool ConfigManager::getDisabledGameCart() {
    return _readBoolean(DISABLED_GAME_CART_KEY, false, _internalDb);
}

bool ConfigManager::getReceivedExFATWarning() {
    return _readBoolean(RECEIVED_EXFAT_WARNING_KEY, false, _internalDb);
}

bool ConfigManager::setInstalledFiles(vector<string> files) {
    return _writeArrayOfStrings(INSTALLED_FILES_KEY, files, _internalDb, INTERNAL_FILENAME);
}

bool ConfigManager::setDisabledGameCart(bool disabled) {
    return _writeBoolean(DISABLED_GAME_CART_KEY, disabled, _internalDb, INTERNAL_FILENAME);
}

bool ConfigManager::setReceivedExFATWarning(bool received) {
    return _writeBoolean(RECEIVED_EXFAT_WARNING_KEY, received, _internalDb, INTERNAL_FILENAME);
}

// Private Methods

bool ConfigManager::_readBoolean(string key, bool def, config_t config) {
    int result;

    if (!config_lookup_bool(&config, key.c_str(), &result))
        return def;

    return result;
}

string ConfigManager::_readString(string key, string def, config_t config) {
    const char * result;

    if (!config_lookup_string(&config, key.c_str(), &result))
        return def;

    return string(result);
}

vector<string> ConfigManager::_readArrayOfStrings(string key, vector<string> def, config_t config) {
    vector<string> result;
    
    config_setting_t * array = config_lookup(&config, key.c_str());
    if (array == NULL)
        return def;

    int count = config_setting_length(array);
    for (int i = 0; i < count; i++) {
        result.push_back(string(config_setting_get_string_elem(array, i)));
    }

    return result;
}

bool ConfigManager::_writeBoolean(string key, bool value, config_t config, string filename) {
    config_setting_t * root, * setting;
    root = config_root_setting(&config);

    setting = config_setting_get_member(root, key.c_str());
    if (setting == NULL) {
        setting = config_setting_add(root, key.c_str(), CONFIG_TYPE_BOOL);
    }

    config_setting_set_bool(setting, value);

    return config_write_file(&config, filename.c_str());
}

bool ConfigManager::_writeString(string key, string value, config_t config, string filename) {
    config_setting_t * root, * setting;
    root = config_root_setting(&config);

    setting = config_setting_get_member(root, key.c_str());
    if (setting == NULL) {
        setting = config_setting_add(root, key.c_str(), CONFIG_TYPE_STRING);
    }

    config_setting_set_string(setting, value.c_str());

    return config_write_file(&config, filename.c_str());
}

bool ConfigManager::_writeArrayOfStrings(string key, vector<string> values, config_t config, string filename) {
    config_setting_t * root = config_root_setting(&config);
    config_setting_remove(root, key.c_str());

    config_setting_t * array = config_setting_add(root, key.c_str(), CONFIG_TYPE_ARRAY);
    for (auto const& value : values) {
        config_setting_set_string_elem(array, -1, value.c_str());
    }

    return config_write_file(&config, filename.c_str());
}
