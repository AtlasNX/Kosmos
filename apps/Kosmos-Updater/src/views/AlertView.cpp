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

#include <SDL2/SDL2_gfxPrimitives.h>
#include "AlertView.hpp"
#include "../AssetManager.hpp"
#include "../SceneDirector.hpp"

AlertView::AlertView(string title, string message, vector<string> buttons) {
    _focusSelection = 0;
    _alertHeight = 203;

    _titleTextView = new TextView(AssetManager::header_font, title, AssetManager::text);
    _titleTextView->textAlignment = CENTER_ALIGN;
    _alertHeight += _titleTextView->getTextHeight();
    addSubView(_titleTextView);

    _messageTextView = new TextView(AssetManager::body_font, message, AssetManager::text);
    _messageTextView->textAlignment = CENTER_ALIGN;
    _messageTextView->lineHeight = 32;
    _alertHeight += _messageTextView->getTextHeight();
    addSubView(_messageTextView);
    
    _alertY = (720 - _alertHeight) / 2;
    int i = 0;
    int buttonWidth = 770 / buttons.size();
    for (auto const& text : buttons) {
        AlertButtonView * button = new AlertButtonView(text, (i == 0), { 255 + buttonWidth * i, _alertY + _alertHeight - 72, buttonWidth, 72 });
        button->isLast = (i == (int) buttons.size() - 1);
        _buttons.push_back(button);
        addSubView(button);
        i++;
    }

    _titleTextView->frame = { 255, _alertY + 55, 770, 1 };
    _messageTextView->frame = { 255, _alertY + 93 + _titleTextView->getTextHeight(), 770, 1 };
}

AlertView::~AlertView() {
    if (_titleTextView != NULL)
        delete _titleTextView;

    if (_messageTextView != NULL)
        delete _messageTextView;
    
    for (auto const& button : _buttons) {
        if (button != NULL)
            delete button;
    }
    _buttons.clear();
}

void AlertView::handleButton(u32 buttons) {
    if (buttons & KEY_LEFT && _focusSelection > 0) {
        Mix_PlayChannel(-1, AssetManager::select, 0);
        _focusSelection--;
        int i = 0;
        for (auto const& button : _buttons) {
            button->setHasFocus(i == _focusSelection);
            i++;
        }
    }
    else if (buttons & KEY_RIGHT && _focusSelection < (int) _buttons.size() - 1) {
        Mix_PlayChannel(-1, AssetManager::select, 0);
        _focusSelection++;
        int i = 0;
        for (auto const& button : _buttons) {
            button->setHasFocus(i == _focusSelection);
            i++;
        }
    }
    else if (buttons & KEY_A) {
        Mix_PlayChannel(-1, AssetManager::enter, 0);
        dismiss(true);
    }
    else if (buttons & KEY_B) {
        Mix_PlayChannel(-1, AssetManager::back, 0);
        dismiss(false);
    }
}

void AlertView::render(SDL_Rect rect, double dTime) {
    // Draw background.
    roundedBoxRGBA(
        SceneDirector::renderer,
        255,
        _alertY,
        1025,
        _alertY + _alertHeight, 
        4,
        AssetManager::background.r, AssetManager::background.g, AssetManager::background.b, AssetManager::background.a);

    // Render any subviews.
    View::render(rect, dTime);
}

int AlertView::getSelectedOption() {
    return _focusSelection;
}

void AlertView::reset() {
    _focusSelection = 0;
    int i = 0;
    for (auto const& button : _buttons) {
        button->setHasFocus(i == _focusSelection);
        i++;
    }
}
