#pragma once

#include <vector>
#include <string>

class IniOption {
    public:
        std::string key;
        std::string value;

        IniOption(std::string name, std::string val);
        std::string build();
        static IniOption * parse(std::string line);
};
