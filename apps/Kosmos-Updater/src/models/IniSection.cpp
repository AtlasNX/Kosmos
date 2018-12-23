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

#include "IniSection.hpp"
#include <algorithm>

IniSection::IniSection(bool isCaption, bool isComment, std::string val) {
    _isCaption = isCaption;
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

bool IniSection::isCaption() {
    return _isCaption;
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
    if (_isCaption)
        return "{" + value + "}\n";
        
    if (_isComment)
        return "#" + value + "\n";

    std::string result = "[" + value + "]\n";

    for (IniOption * option : options) {
        result += option->build() + "\n";
    }

    return result;
}

IniSection * IniSection::parse(std::string line) {
    if (line.at(0) == '{' && line.at(line.size() - 1) == '}') {
        return new IniSection(true, false, line.substr(1, line.size() - 2));
    } else if (line.at(0) == '#') {
        return new IniSection(false, true, line.substr(1, line.size() - 1));
    } else if (line.at(0) == '[' && line.at(line.size() - 1) == ']') {
        return new IniSection(false, false, line.substr(1, line.size() - 2));
    }  else {
        return nullptr;
    }
}
