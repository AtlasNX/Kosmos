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

#include "AppUpdateScene.hpp"
#include "../SceneDirector.hpp"
#include "../NetManager.hpp"
#include "../FileManager.hpp"

AppUpdateScene::AppUpdateScene() {
    _headerView = new HeaderView("Kosmos Updater", true);
    _headerView->frame = { 0, 0, 1280, 88 };

    _updateView = new UpdateView("Checking for updates to Kosmos Updater...");
    _updateView->frame.x = 0;
    _updateView->frame.y = 200;

    _statusView = new StatusView("", "");
    _statusView->frame.x = 0;
    _statusView->frame.y = 323;
    _statusView->hidden = true;

    _footerView = new FooterView();
    _footerView->frame = { 0, 647, 1280, 73 };

    addSubView(_headerView);
    addSubView(_updateView);
    addSubView(_statusView);
    addSubView(_footerView);

    _versionRequest = NetManager::getLatestAppVersion();
    _appRequest = NULL;
}

AppUpdateScene::~AppUpdateScene() {
    if (_versionRequest != NULL)
        delete _versionRequest;

    if (_appRequest != NULL)
        delete _appRequest;

    if (_headerView != NULL)
        delete _headerView;

    if (_updateView != NULL)
        delete _updateView;

    if (_statusView != NULL)
        delete _statusView;

    if (_footerView != NULL)
        delete _footerView;
}

void AppUpdateScene::handleButton(u32 buttons) {
    if (!_statusView->hidden && buttons & KEY_A) {
        SceneDirector::exitApp = true;
    }
}

void AppUpdateScene::render(SDL_Rect rect, double dTime) {
    if (_versionRequest != NULL) {
        _updateVersionRequest();
    }
    else if (_appRequest != NULL) {
        _updateAppRequest();
    }

    Scene::render(rect, dTime);
}

void AppUpdateScene::_updateVersionRequest() {
    mutexLock(&_versionRequest->mutexRequest);

    _updateView->setProgress(_versionRequest->progress);
    if (_versionRequest->isComplete) {
        _latestAppVersion = string(_versionRequest->getData());
        _parseLatestAppVersion();

        // No Update
        if (
            VERSION_MAJOR > _latestAppMajorVersion ||
            (VERSION_MAJOR == _latestAppMajorVersion && VERSION_MINOR > _latestAppMinorVersion) ||
            (VERSION_MAJOR == _latestAppMajorVersion && VERSION_MINOR == _latestAppMinorVersion && VERSION_PATCH > _latestAppPatchVersion) ||
            (VERSION_MAJOR == _latestAppMajorVersion && VERSION_MINOR == _latestAppMinorVersion && VERSION_PATCH == _latestAppPatchVersion)
        ) {
            SceneDirector::currentScene = SCENE_PACKAGE_SELECT;
        }
        // Update
        else {
            delete _versionRequest;
            _versionRequest = NULL;

            _updateView->setProgress(0);
            _updateView->setText("Getting the latest version of Kosmos Updater...");

            _appRequest = NetManager::getLatestApp();
        }
    }
    else if (_versionRequest->hasError) {
        _showStatus(_versionRequest->errorMessage, "Please restart the app to try again.");

        delete _versionRequest;
        _versionRequest = NULL;
    }

    if (_versionRequest != NULL)
        mutexUnlock(&_versionRequest->mutexRequest);
}

void AppUpdateScene::_updateAppRequest() {
    mutexLock(&_appRequest->mutexRequest);
    
    _updateView->setProgress(_appRequest->progress);
    if (_appRequest->isComplete) {
        romfsExit();
        FileManager::writeFile("KosmosUpdater.nro", _appRequest);

        delete _appRequest;
        _appRequest = NULL;

        _showStatus("Kosmos Updater has been updated to version " + _latestAppVersion + "!", "Please restart the app to update your files.");
    }
    else if (_appRequest->hasError) {
        _showStatus(_appRequest->errorMessage, "Please restart the app to try again.");

        delete _appRequest;
        _appRequest = NULL;
    }

    if (_appRequest != NULL)
        mutexUnlock(&_appRequest->mutexRequest);
}

void AppUpdateScene::_showStatus(string text, string subtext) {
    _statusView->setText(text);
    _statusView->setSubtext(subtext);

    _updateView->hidden = true;
    _statusView->hidden = false;

    _footerView->actions.push_back(new Action(A_BUTTON, "Quit"));
}

void AppUpdateScene::_parseLatestAppVersion() {
    size_t pos = 0;
    int index = 0;
    while (pos != string::npos) {
        size_t end_pos = _latestAppVersion.find(".", pos);

        int versionNumber = 0;
        if (end_pos == string::npos) {
            versionNumber = stoi(_latestAppVersion.substr(pos, string::npos));
            pos = string::npos;
        } else {
            versionNumber = stoi(_latestAppVersion.substr(pos, end_pos - pos));
            pos = end_pos + 1;
        }

        if (index == 0) {
            _latestAppMajorVersion = versionNumber;
        } else if (index == 1) {
            _latestAppMinorVersion = versionNumber;
        } else if (index == 2) {
            _latestAppPatchVersion = versionNumber;
        }

        index++;
    }
}
