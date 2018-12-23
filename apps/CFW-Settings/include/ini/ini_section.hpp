#pragma once

#include <vector>
#include <string>

#include "ini_option.hpp"

class IniSection {
    public:
        std::string value;
        std::vector<IniOption *> options;

        IniSection(bool isComment, std::string val);
        ~IniSection();
        bool isComment();
        IniOption * findFirstOption(std::string key);
        std::string build();
        static IniSection * parse(std::string line);

    private:
        bool _isComment;
};
