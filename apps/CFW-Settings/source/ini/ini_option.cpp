#include "ini_option.hpp"
#include "trim.hpp"

IniOption::IniOption(std::string name, std::string val) {
    key = name;
    value = val;
}

std::string IniOption::build() {
    return key + '=' + value;
}

IniOption * IniOption::parse(std::string line) {
    size_t pos = line.find('=');
    if (pos != std::string::npos && pos > 0) {
        return new IniOption(line.substr(0, pos), line.substr(pos + 1));
    } else {
        return nullptr;
    }
}
