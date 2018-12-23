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

#include <string>
#include <vector>
#include "../ModalView.hpp"
#include "HeaderView.hpp"
#include "ListRowView.hpp"
#include "FooterView.hpp"

using namespace std;

class MultiSelectView : public ModalView {
    public:
        MultiSelectView(string title, vector<string> options, string selectedOption);
        ~MultiSelectView();

        void handleButton(u32 buttons);
        void render(SDL_Rect rect, double dTime);
        string getSelectedOption();
        void reset(string selectedOption);

    private:
        int _startY;
        long unsigned int _focusSelection;
        vector<string> _options;

        HeaderView * _headerView;
        vector<ListRowView *> _listRowViews;
        FooterView * _footerView;

        bool _goUp();
        bool _goDown();
};
