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

#include "StatusView.hpp"
#include "../AssetManager.hpp"

StatusView::StatusView(string text, string subtext) : View() {
    frame = { 0, 0, 1280, 100 };

    _textView = new TextView(AssetManager::body_font, text, AssetManager::text);
    _textView->frame = { 0, 0, 1280, 1 };
    _textView->textAlignment = CENTER_ALIGN;

    _subtextView = new TextView(AssetManager::subbody_font, subtext, AssetManager::text);
    _subtextView->frame = { 0, 52, 1280, 1 };
    _subtextView->textAlignment = CENTER_ALIGN;

    addSubView(_textView);
    addSubView(_subtextView);
}

StatusView::~StatusView() {
    if (_textView == NULL)
        delete _textView;

    if (_subtextView == NULL)
        delete _subtextView;
}

void StatusView::render(SDL_Rect rect, double dTime) {
    // Render any subviews.
    View::render(rect, dTime);
}

void StatusView::setText(string text) {
    _textView->setText(text);
}

void StatusView::setSubtext(string text) {
    _subtextView->setText(text);
}
