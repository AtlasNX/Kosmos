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

#include "../Scene.hpp"
#include "../models/NetRequest.hpp"
#include "../models/ThreadObj.hpp"
#include "../models/Zip.hpp"
#include "../views/HeaderView.hpp"
#include "../views/UpdateView.hpp"
#include "../views/StatusView.hpp"
#include "../views/FooterView.hpp"
#include "../views/AlertView.hpp"
#include "../ModalView.hpp"

class PackageDownloadScene : public Scene {
    public:
        PackageDownloadScene();
        ~PackageDownloadScene();

        void handleButton(u32 buttons);
        void render(SDL_Rect rect, double dTime);

    private:
        ThreadObj * _packageDelete;
        NetRequest * _packageRequest;
        Zip * _packageExtract;
        ThreadObj * _packageDisableGC;
        string _versionNumber;
        int _numberOfFiles;

        HeaderView * _headerView;
        UpdateView * _updateView;
        StatusView * _statusView;
        FooterView * _footerView;

        AlertView * _restartAlertView;

        void _updatePackageDelete();
        void _updatePackageRequest();
        void _updatePackageExtract();
        void _updatePackageDisableGC();
        void _showStatus(string text, string subtext, bool wasSuccessful);
        void _onAlertViewDismiss(ModalView * view, bool success);
};
