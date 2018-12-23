#pragma once

#include <vector>
#include <string>

#include "ini_section.hpp"

class Ini {
    public:
        std::vector<IniSection *> sections;

        ~Ini();
        std::string build();
        IniSection * findSection(std::string name);
        bool writeToFile(std::string path);
        static Ini * parseFile(std::string path);
};
