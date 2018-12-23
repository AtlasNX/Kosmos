#include "ini_section.hpp"
#include "trim.hpp"

IniSection::IniSection(bool isComment, std::string val) {
    _isComment = isComment;
    value = val;
}

IniSection::~IniSection() {
    for (IniOption * option : options) {
        if (option != nullptr) {
            delete option;
            option = nullptr;
        }
    }
    options.clear();
}

bool IniSection::isComment() {
    return _isComment;
}

IniOption * IniSection::findFirstOption(std::string key) {
    auto it = std::find_if(options.begin(), options.end(), [&key](const IniOption * obj) { return obj->key == key; });
    if (it == options.end())
        return nullptr;

    return (*it);
}

std::string IniSection::build() {
    if (_isComment)
        return "{" + value + "}\n";

    std::string result = "[" + value + "]\n";

    for (IniOption * option : options) {
        result += option->build() + "\n";
    }

    return result;
}

IniSection * IniSection::parse(std::string line) {
    if (line.at(0) == '[' && line.at(line.size() - 1) == ']') {
        return new IniSection(false, line.substr(1, line.size() - 2));
    } else if (line.at(0) == '{' && line.at(line.size() - 1) == '}') {
        return new IniSection(true, line.substr(1, line.size() - 2));
    } else {
        return nullptr;
    }
}
