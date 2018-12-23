#include <fstream>
#include <iostream>
#include "ini.hpp"
#include "ini_option.hpp"
#include "trim.hpp"

Ini::~Ini() {
    for (IniSection * section : sections) {
        if (section != nullptr) {
            delete section;
            section = nullptr;
        }
    }
    sections.clear();
}

std::string Ini::build() {
    std::string result;

    for (IniSection * section : sections) {
        result += section->build() + "\n";
    }

    return result;
}

IniSection * Ini::findSection(std::string name) {
    auto it = std::find_if(sections.begin(), sections.end(), [&name](const IniSection * obj) { return obj->value == name; });
    if (it == sections.end())
        return nullptr;

    return (*it);
}

bool Ini::writeToFile(std::string path) {
    std::ofstream file(path);
    if (!file.is_open())
        return false;

    file << build();

    file.close();

    return true;
}

Ini * Ini::parseFile(std::string path) {
    std::ifstream file(path);
    if (!file.is_open())
        return nullptr;

    Ini * ini = new Ini();
    std::string line;
    while (getline(file, line)) {
        trim(line);

        if (line.size() == 0)
            continue;

        if (line.at(0) == '[' || line.at(0) == '{') {
            IniSection * section = IniSection::parse(line);

            if (section != nullptr)
                ini->sections.push_back(section);
        } else if (ini->sections.size() != 0 && ini->sections.back()->isComment() == false) {
            IniOption * option = IniOption::parse(line);

            if (option != nullptr)
                ini->sections.back()->options.push_back(option);
        }
    }

    file.close();

    return ini;
}
