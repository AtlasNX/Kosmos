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

#include "AlertButtonView.hpp"
#include "../AssetManager.hpp"
#include "../SceneDirector.hpp"

AlertButtonView::AlertButtonView(string title, bool hasFocus, SDL_Rect rect) {
    frame = rect;

    _textView = new TextView(AssetManager::body_font, title, (hasFocus) ? AssetManager::active_text : AssetManager::text);
    _textView->textAlignment = CENTER_ALIGN;
    _textView->frame = { 0, (72 - _textView->getTextHeight()) / 2, rect.w, 1 };

    addSubView(_textView);
}

AlertButtonView::~AlertButtonView() {
    if (_textView != NULL)
        delete _textView;
}

void AlertButtonView::setHasFocus(bool focus) {
    _textView->setTextColor((focus) ? AssetManager::active_text : AssetManager::text);
}

void AlertButtonView::render(SDL_Rect rect, double dTime) {
    AssetManager::setRenderColor(AssetManager::list_divider);
    SDL_RenderDrawLine(SceneDirector::renderer, rect.x, rect.y, rect.x + rect.w, rect.y);
    if (!isLast) {
        SDL_RenderDrawLine(SceneDirector::renderer, rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h);
    }

    // Render any subviews.
    ControlView::render(rect, dTime);
}