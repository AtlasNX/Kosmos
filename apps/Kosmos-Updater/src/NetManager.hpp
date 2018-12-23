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

#include <curl/curl.h>
#include <switch.h>
#include <string>
#include <vector>
#include "models/NetRequest.hpp"

using namespace std;

class NetManager {
    public:
        static void initialize();
        static void dealloc();

        static NetRequest * getLatestAppVersion();
        static NetRequest * getLatestApp();
        static NetRequest * getLatestKosmosVersion(string channel);
        static NetRequest * getLatestKosmos(string bundle, string channel);

    private:
        static inline bool _shouldUseProxy;
        static inline string _proxyURL;
        static inline string _proxyUsername;
        static inline string _proxyPassword;
        static inline string _hostname;
        static inline vector<Thread> _threads;
        
        static Result _createThread(ThreadFunc func, NetRequest * ptr);
        static void _request(void * ptr);
        static int _progressFunction(void *ptr, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
        static size_t _writeFunction(void *contents, size_t size, size_t nmemb, void * ptr);
        static size_t _headerFunction(void *contents, size_t size, size_t nmemb, void * ptr);
};
