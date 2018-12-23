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

#include "ListHeaderView.hpp"
#include "../SceneDirector.hpp"

ListHeaderView::ListHeaderView(string text) : View() {
    frame = { 0, 0, 840, 80 };

    _textView = new TextView(AssetManager::subbody_font, text, AssetManager::text);
    _textView->frame = { 20, 47, 800, 0 };

    addSubView(_textView);
}

ListHeaderView::~ListHeaderView() {
    if (_textView != NULL)
        delete _textView;
}

void ListHeaderView::render(SDL_Rect rect, double dTime) {
    // Draw rectangle.
    AssetManager::setRenderColor(AssetManager::list_divider);
    SDL_Rect squareFrame = { rect.x + 5, rect.y + 44, 5, 22 };
    SDL_RenderFillRect(SceneDirector::renderer, &squareFrame);

    // Render any subviews.
    View::render(rect, dTime);
}

void ListHeaderView::setText(string text) {
    _textView->setText(text);
}
