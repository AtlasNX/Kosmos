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

#pragma once

#include <switch.h>
#include <string>

#include "ThreadObj.hpp"

using namespace std;

class NetRequest : public ThreadObj {
    public:
        string bundle;
        string channel;

        NetRequest(string method, string url);
        ~NetRequest();
        string getMethod();
        string getURL();
        size_t getSize();
        char * getData();
        string getVersionNumber();
        string getNumberOfFiles();
        size_t appendData(void *contents, size_t size, size_t nmemb);
        size_t appendHeaderData(void *contents, size_t size, size_t nmemb);

    private:
        string _method;
        string _url;
        size_t _size;
        char * _data;
        size_t _headerSize;
        char * _headerData;

        string _versionNumber;
        string _numberOfFiles;
};
