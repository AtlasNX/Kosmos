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

#include <string.h>
#include "NetManager.hpp"
#include "ConfigManager.hpp"
#include "models/NetRequest.hpp"

void NetManager::initialize() {
    socketInitializeDefault();
    #ifdef DEBUG
        nxlinkStdio();
    #endif
    
    curl_global_init(CURL_GLOBAL_ALL);
    _hostname = ConfigManager::getHost();
    _shouldUseProxy = ConfigManager::shouldUseProxy();
    _proxyURL = ConfigManager::getProxy();
    _proxyUsername = ConfigManager::getProxyUsername();
    _proxyPassword = ConfigManager::getProxyPassword();
}

void NetManager::dealloc() {
    curl_global_cleanup();
    socketExit();
}

NetRequest * NetManager::getLatestAppVersion() {
    NetRequest * request = new NetRequest("GET", _hostname + "/" + API_VERSION + "/app/version-number");
    _createThread(_request, request);
    return request;
}

NetRequest * NetManager::getLatestApp() {
    NetRequest * request = new NetRequest("GET", _hostname + "/" + API_VERSION + "/app");
    _createThread(_request, request);
    return request;
}

NetRequest * NetManager::getLatestKosmosVersion(string channel) {
    NetRequest * request = new NetRequest("GET", _hostname + "/" + API_VERSION + "/package/version-number");
    request->channel = channel;
    _createThread(_request, request);
    return request;
}

NetRequest * NetManager::getLatestKosmos(string bundle, string channel) {
    NetRequest * request = new NetRequest("GET", _hostname + "/" + API_VERSION + "/package");
    request->bundle = bundle;
    request->channel = channel;
    _createThread(_request, request);
    return request;
}

Result NetManager::_createThread(ThreadFunc func, NetRequest * request) {
    Thread thread;
    Result res;

    if (R_FAILED( res = threadCreate(&thread, func, (void *) request, 0x2000, 0x2B, -2)))
        return res;
    if (R_FAILED( res = threadStart(&thread)))
        return res;

    request->thread = thread;
    _threads.push_back(thread);

    return 0;
}

void NetManager::_request(void * ptr) {
    NetRequest * request = (NetRequest *) ptr;
    CURL * curl;
    CURLcode res;
    string userAgent = string("kosmos-updater/") + VERSION;

    curl = curl_easy_init();

    if (curl) {
        string url = request->getURL();
        if (request->bundle != "" || request->channel != "") {
            url += "?";
            if (request->bundle != "") {
                url += "bundle=" + string(curl_easy_escape(curl, request->bundle.c_str(), 0));
                
                if (request->channel != "") {
                    url += "&";
                }
            }
            
            if (request->channel != "") {
                url += "channel=" + string(curl_easy_escape(curl, request->channel.c_str(), 0));
            }
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request->getMethod().c_str());
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION , _headerFunction);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *) request);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) request);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, _progressFunction);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void *) request);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());

        if (_shouldUseProxy && _proxyURL.size() > 0) {
            curl_easy_setopt(curl, CURLOPT_PROXY, _proxyURL.c_str());

            if (_proxyUsername.size() > 0)
                curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, _proxyUsername.c_str());
                
            if (_proxyPassword.size() > 0)
                curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, _proxyPassword.c_str());
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            mutexLock(&request->mutexRequest);

            request->hasError = true;
            request->errorMessage = string(curl_easy_strerror(res));

            mutexUnlock(&request->mutexRequest);

            curl_easy_cleanup(curl);
            return;
        }

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            mutexLock(&request->mutexRequest);

            request->hasError = true;
            request->errorMessage = "There was an error on the server, please try again later.";

            mutexUnlock(&request->mutexRequest);

            curl_easy_cleanup(curl);
            return;
        }

        curl_easy_cleanup(curl);
    }

    mutexLock(&request->mutexRequest);

    request->isComplete = true;

    mutexUnlock(&request->mutexRequest);
}

int NetManager::_progressFunction(void *ptr, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    NetRequest * request = (NetRequest *) ptr;
    mutexLock(&request->mutexRequest);

    request->progress = (double) dlnow / (double) dltotal;

    mutexUnlock(&request->mutexRequest);
    return 0;
}

size_t NetManager::_writeFunction(void *contents, size_t size, size_t nmemb, void * ptr) {
    NetRequest * request = (NetRequest *) ptr;
    return request->appendData(contents, size, nmemb);
}

size_t NetManager::_headerFunction(void *contents, size_t size, size_t nmemb, void * ptr) {
    NetRequest * request = (NetRequest *) ptr;
    return request->appendHeaderData(contents, size, nmemb);
}
