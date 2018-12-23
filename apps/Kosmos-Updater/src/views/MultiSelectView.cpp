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

#include "MultiSelectView.hpp"
#include "../AssetManager.hpp"
#include "../SceneDirector.hpp"

MultiSelectView::MultiSelectView(string title, vector<string> options, string selectedOption) : ModalView() {
    _startY = 720 - (224 + min((int) options.size(), 4) * 71);
    _focusSelection = 0;
    _options = options;

    for (auto const& option : options) {
        ListRowView * rowView = new ListRowView(option, "", DEFAULT);
        rowView->frame.x = 215;
        rowView->frame.y = _startY + 120 + (_listRowViews.size() * 71);
        rowView->hasFocus = _listRowViews.size() == 0;
        rowView->hasCheckmark = option.compare(selectedOption) == 0;

        _listRowViews.push_back(rowView);
        addSubView(rowView);
    }
    _listRowViews.back()->isLast = true;

    _headerView = new HeaderView(title.c_str(), false);
    _headerView->frame = { 0, _startY, 1280, 88 };
    addSubView(_headerView);

    _footerView = new FooterView();
    _footerView->frame = { 0, 647, 1280, 73 };
    _footerView->actions.push_back(new Action(A_BUTTON, "OK"));
    _footerView->actions.push_back(new Action(B_BUTTON, "Back"));
    addSubView(_footerView);
}

MultiSelectView::~MultiSelectView() {
    if (_headerView != NULL)
        delete _headerView;

    for (auto const& rowView : _listRowViews) {
        if (rowView != NULL)
            delete rowView;
    }
    _listRowViews.clear();

    if (_footerView != NULL)
        delete _footerView;
}

void MultiSelectView::handleButton(u32 buttons) {
    if (buttons & KEY_A) {
        Mix_PlayChannel(-1, AssetManager::enter, 0);
        dismiss(true);
    }

    if (buttons & KEY_B) {
        Mix_PlayChannel(-1, AssetManager::back, 0);
        dismiss(false);
    }

    if (buttons & KEY_UP && _goUp()) {
        Mix_PlayChannel(-1, AssetManager::select, 0);
    }

    if (buttons & KEY_DOWN && _goDown()) {
        Mix_PlayChannel(-1, AssetManager::select, 0);
    }
}

void MultiSelectView::render(SDL_Rect rect, double dTime) {
    // Draw background.
    AssetManager::setRenderColor(AssetManager::background);
    SDL_Rect bgFrame = { 0, _startY, 1280, 720 - _startY };
    SDL_RenderFillRect(SceneDirector::renderer, &bgFrame);

    // Render any subviews.
    View::render(rect, dTime);
}

string MultiSelectView::getSelectedOption() {
    return _options[_focusSelection];
}

void MultiSelectView::reset(string selectedOption) {
    _focusSelection = 0;

    for (long unsigned int i = 0; i < _listRowViews.size(); i++) {
        _listRowViews[i]->hasFocus = (i == 0);
        _listRowViews[i]->hasCheckmark = _options[i].compare(selectedOption) == 0;
    }
}

bool MultiSelectView::_goUp() {
    if (_focusSelection != 0) {
        _focusSelection--;

        int index = 0;
        for (auto const& rowView : _listRowViews) {
            rowView->hasFocus = false;

            if (_focusSelection >= 3) {
                rowView->frame.y = (_startY + 120 + index * 71) - ((_focusSelection - 3) * 71);
            }

            index++;
        }

        _listRowViews[_focusSelection]->hasFocus = true;
        return true;
    }

    return false;
}

bool MultiSelectView::_goDown() {
    if (_focusSelection != _listRowViews.size() -1) {
        _focusSelection++;
        
        int index = 0;
        for (auto const& rowView : _listRowViews) {
            rowView->hasFocus = false;

            if (_focusSelection >= 3) {
                rowView->frame.y = (_startY + 120 + index * 71) - ((_focusSelection - 3) * 71);
            }

            index++;
        }

        _listRowViews[_focusSelection]->hasFocus = true;

        return true;
    }

    return false;
}
