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

#include "PackageSelectScene.hpp"
#include "../SceneDirector.hpp"
#include "../NetManager.hpp"
#include "../ConfigManager.hpp"

using namespace std::placeholders;

PackageSelectScene::PackageSelectScene() {
    _channelOpen = false;
    _bundleOpen = false;
    _focusSelection = 0;

    _headerView = new HeaderView("Kosmos Updater", true);
    _headerView->frame = { 0, 0, 1280, 88 };

    _updateView = new UpdateView("Checking for updates to Kosmos...");
    _updateView->frame.x = 0;
    _updateView->frame.y = 200;

    _statusView = new StatusView("", "");
    _statusView->frame.x = 0;
    _statusView->frame.y = 323;

    _installRowView = new ListRowView("Install Latest Kosmos", "", SUBTITLE);
    _installRowView->frame.x = 215;
    _installRowView->frame.y = 137;
    _installRowView->isLast = true;
    _installRowView->hasFocus = true;

    _settingHeaderView = new ListHeaderView("Settings");
    _settingHeaderView->frame.x = 220;
    _settingHeaderView->frame.y = 208;

    string channel = ConfigManager::getChannel();
    if (channel == "bleeding-edge") {
        _channelSelected = "Bleeding Edge";
    } else {
        _channelSelected = "Stable";
    }

    _channelRowView = new ListRowView("Channel", _channelSelected, VALUE);
    _channelRowView->frame.x = 215;
    _channelRowView->frame.y = 288;

    string bundle = ConfigManager::getBundle();
    if (bundle == "atmosphere") {
        _bundleSelected = "Atmosphere";
    } else {
        _bundleSelected = "Kosmos";
    }

    _bundleRowView = new ListRowView("Bundle", _bundleSelected, VALUE);
    _bundleRowView->frame.x = 215;
    _bundleRowView->frame.y = 359;

    _disabledGameCart = ConfigManager::getDisabledGameCart();
    _disableGCRowView = new ListRowView("Disable Game Cart", "", BOOLEAN);
    _disableGCRowView->frame.x = 215;
    _disableGCRowView->frame.y = 430;
    _disableGCRowView->isLast = true;
    _disableGCRowView->setIsOn(_disabledGameCart);

    _footerView = new FooterView();
    _footerView->frame = { 0, 647, 1280, 73 };

    vector<string> channelOptions;
    channelOptions.push_back("Stable");
    channelOptions.push_back("Bleeding Edge");

    _channelMultiSelectView = new MultiSelectView("Channel", channelOptions, _channelSelected);
    _channelMultiSelectView->onDismiss = bind(&PackageSelectScene::_onMultiSelectDismiss, this, _1, _2);

    vector<string> bundleOptions;
    bundleOptions.push_back("Kosmos");
    bundleOptions.push_back("Atmosphere");

    _bundleMultiSelectView = new MultiSelectView("Bundle", bundleOptions, _bundleSelected);
    _bundleMultiSelectView->onDismiss = bind(&PackageSelectScene::_onMultiSelectDismiss, this, _1, _2);

    vector<string> buttons;
    buttons.push_back("Yes");
    buttons.push_back("No");
    _disabledGameCartAlertView = new AlertView("Are you sure?", "Enabling the game cart after you have updated with\nChoiDujourNX could cause your game cart firmware to\nupgrade, if this happens there is no way to\ndowngrade it later.", buttons);
    _disabledGameCartAlertView->onDismiss = bind(&PackageSelectScene::_onAlertViewDismiss, this, _1, _2);

    addSubView(_headerView);
    addSubView(_updateView);
    addSubView(_statusView);
    addSubView(_installRowView);
    addSubView(_settingHeaderView);
    addSubView(_channelRowView);
    addSubView(_bundleRowView);
    addSubView(_disableGCRowView);
    addSubView(_footerView);

    _showUpdateView();
    _versionRequest = NetManager::getLatestKosmosVersion(channel);
}

PackageSelectScene::~PackageSelectScene() {
    if (_headerView != NULL)
        delete _headerView;

    if (_updateView != NULL)
        delete _updateView;

    if (_statusView != NULL)
        delete _statusView;

    if (_installRowView != NULL)
        delete _installRowView;

    if (_settingHeaderView != NULL)
        delete _settingHeaderView;

    if (_channelRowView != NULL)
        delete _channelRowView;

    if (_bundleRowView != NULL)
        delete _bundleRowView;

    if (_footerView != NULL)
        delete _footerView;

    if (_channelMultiSelectView != NULL)
        delete _channelMultiSelectView;

    if (_bundleMultiSelectView != NULL)
        delete _bundleMultiSelectView;

    if (_disabledGameCartAlertView != NULL)
        delete _disabledGameCartAlertView;

    if (_versionRequest != NULL)
        delete _versionRequest;
}

void PackageSelectScene::handleButton(u32 buttons) {
    if (!_statusView->hidden && buttons & KEY_A) {
        Mix_PlayChannel(-1, AssetManager::back, 0);
        SceneDirector::exitApp = true;
    }
    else if (_updateView->hidden && _statusView->hidden) {
        if (buttons & KEY_A) {
            Mix_PlayChannel(-1, AssetManager::enter, 0);

            switch(_focusSelection) {
                case 0:
                    SceneDirector::currentScene = SCENE_PACKAGE_DOWNLOAD;
                    break;

                case 1:
                    _focusSelection = -1;
                    _manageFocus();

                    _channelMultiSelectView->show();
                    break;
                
                case 2:
                    _focusSelection = -1;
                    _manageFocus();

                    _bundleMultiSelectView->show();
                    break;

                case 3:
                    if (_disabledGameCart) {
                        _disabledGameCartAlertView->reset();
                        _disabledGameCartAlertView->show();
                    } else {
                        _disabledGameCart = !_disabledGameCart;
                        _disableGCRowView->setIsOn(_disabledGameCart);
                        ConfigManager::setDisabledGameCart(_disabledGameCart);
                    }

                    break;
            }
        }

        if (buttons & KEY_B) {
            Mix_PlayChannel(-1, AssetManager::back, 0);
            SceneDirector::exitApp = true;
        }

        if (buttons & KEY_UP && _focusSelection != 0) {
            Mix_PlayChannel(-1, AssetManager::select, 0);
            _focusSelection--;
            _manageFocus();
        }

        if (buttons & KEY_DOWN && _focusSelection != 3) {
            Mix_PlayChannel(-1, AssetManager::select, 0);
            _focusSelection++;
            _manageFocus();
        }
    }
}

void PackageSelectScene::render(SDL_Rect rect, double dTime) {
    if (_versionRequest != NULL) {
        _updateVersionRequest();
    }

    Scene::render(rect, dTime);
}

void PackageSelectScene::_updateVersionRequest() {
    mutexLock(&_versionRequest->mutexRequest);

    _updateView->setProgress(_versionRequest->progress);
    if (_versionRequest->isComplete) {
        _latestVersion = string(_versionRequest->getData());

        _showPackageSelectViews();

        delete _versionRequest;
        _versionRequest = NULL;
    }
    else if (_versionRequest->hasError) {
        _showStatusView(_versionRequest->errorMessage, "Please restart the app to try again.");

        delete _versionRequest;
        _versionRequest = NULL;
    }

    if (_versionRequest != NULL)
        mutexUnlock(&_versionRequest->mutexRequest);
}

void PackageSelectScene::_showUpdateView() {
    _updateView->setProgress(0);
    _updateView->hidden = false;
    _statusView->hidden = true;
    _installRowView->hidden = true;
    _settingHeaderView->hidden = true;
    _channelRowView->hidden = true;
    _bundleRowView->hidden = true;

    for (auto const& action : _footerView->actions) {
        delete action;
    }
    _footerView->actions.clear();
}

void PackageSelectScene::_showPackageSelectViews() {
    _updateView->hidden = true;
    _statusView->hidden = true;

    _focusSelection = 0;

    _installRowView->hidden = false;
    _installRowView->hasFocus = true;

    string version = ConfigManager::getCurrentVersion();
    if (version.compare(_latestVersion) == 0) {
        _installRowView->setPrimaryText("Reinstall Kosmos");
    } else {
        _installRowView->setPrimaryText("Install Latest Kosmos");
    }

    if (version == "" || version.compare(_latestVersion) == 0) {
        _installRowView->setSecondaryText("Latest Version is " + ((_latestVersion.size() > 7) ? _latestVersion.substr(0, 7) : _latestVersion));
    } else {
        _installRowView->setSecondaryText("You currently have version " + ((version.size() > 7) ? version.substr(0, 7) : version) + " installed, and the latest version is " + ((_latestVersion.size() > 7) ? _latestVersion.substr(0, 7) : _latestVersion + "."));
    }

    _settingHeaderView->hidden = false;
    _settingHeaderView->hasFocus = false;

    _channelRowView->hidden = false;
    _channelRowView->hasFocus = false;

    _bundleRowView->hidden = false;
    _bundleRowView->hasFocus = false;

    for (auto const& action : _footerView->actions) {
        delete action;
    }
    _footerView->actions.clear();
    _footerView->actions.push_back(new Action(A_BUTTON, "OK"));
    _footerView->actions.push_back(new Action(B_BUTTON, "Quit"));
}

void PackageSelectScene::_showStatusView(string text, string subtext) {
    _statusView->setText(text);
    _statusView->setSubtext(subtext);

    _updateView->hidden = true;
    _statusView->hidden = false;
    _installRowView->hidden = true;
    _settingHeaderView->hidden = true;
    _channelRowView->hidden = true;
    _bundleRowView->hidden = true;

    for (auto const& action : _footerView->actions) {
        delete action;
    }
    _footerView->actions.clear();
    _footerView->actions.push_back(new Action(A_BUTTON, "Quit"));
}

void PackageSelectScene::_manageFocus() {
    _installRowView->hasFocus = false;
    _channelRowView->hasFocus = false;
    _bundleRowView->hasFocus = false;
    _disableGCRowView->hasFocus = false;

    switch(_focusSelection) {
        case 0:
            _installRowView->hasFocus = true;
            break;
        
        case 1:
            _channelRowView->hasFocus = true;
            break;

        case 2:
            _bundleRowView->hasFocus = true;
            break;

        case 3:
            _disableGCRowView->hasFocus = true;
            break;
    }
}

void PackageSelectScene::_onMultiSelectDismiss(ModalView * view, bool success) {
    if (success) {
        if (view == _channelMultiSelectView) {
            string channel = _channelMultiSelectView->getSelectedOption();
            if (channel == "Bleeding Edge") {
                ConfigManager::setChannel("bleeding-edge");
            } else {
                ConfigManager::setChannel("stable");
            }

            if (channel.compare(_channelSelected) != 0) {
                _resetVersion(true);
            }

            _channelSelected = channel;
            _channelRowView->setSecondaryText(_channelSelected);
            _channelMultiSelectView->reset(_channelSelected);
        } else {
            string bundle = _bundleMultiSelectView->getSelectedOption();
            if (bundle == "Atmosphere") {
                ConfigManager::setBundle("atmosphere");
            } else {
                ConfigManager::setBundle("kosmos");
            }

            if (bundle.compare(_bundleSelected) != 0) {
                _resetVersion(false);
            }

            _bundleSelected = bundle;
            _bundleRowView->setSecondaryText(_bundleSelected);
            _bundleMultiSelectView->reset(_bundleSelected);
        }
    }

    _focusSelection = (view == _channelMultiSelectView) ? 1 : 2;
    _manageFocus();
}

void PackageSelectScene::_onAlertViewDismiss(ModalView * view, bool success) {
    if (success && _disabledGameCartAlertView->getSelectedOption() == 0) {
        _disabledGameCart = false;
        _disableGCRowView->setIsOn(false);
        ConfigManager::setDisabledGameCart(false);
    }
}

void PackageSelectScene::_resetVersion(bool channelChange) {
    ConfigManager::setCurrentVersion("");

    if (channelChange) {
        _showUpdateView();

        string channel = ConfigManager::getChannel();
        _versionRequest = NetManager::getLatestKosmosVersion(channel);
    }
}
