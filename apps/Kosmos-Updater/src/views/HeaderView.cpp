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

#include "HeaderView.hpp"
#include "../AssetManager.hpp"
#include "../SceneDirector.hpp"

HeaderView::HeaderView(string title, bool showIcon) : View() {
    isFocusable = false;
    isTouchable = false;
    _showIcon = showIcon;

    if (AssetManager::icon == NULL) {
        AssetManager::icon = AssetManager::loadAsset("icon.png");
    }

    SDL_Surface *surface = TTF_RenderText_Blended(AssetManager::header_font, title.c_str(), AssetManager::text);
    _titleTexture = SDL_CreateTextureFromSurface(SceneDirector::renderer, surface);
    _titleWidth = surface->w;
    _titleHeight = surface->h;
    SDL_FreeSurface(surface);
}

HeaderView::~HeaderView() {
    if (_titleTexture != NULL) 
        SDL_DestroyTexture(_titleTexture);
}

void HeaderView::render(SDL_Rect rect, double dTime) {
    // Draw Background
    AssetManager::setRenderColor(AssetManager::background);
    SDL_RenderFillRect(SceneDirector::renderer, &rect);

    if (_showIcon) {
        // Icon
        SDL_Rect iconFrame = { rect.x + 74, rect.y + 29, 30, 44 };
        SDL_RenderCopy(SceneDirector::renderer, AssetManager::icon, NULL, &iconFrame);

        // Title
        SDL_Rect titleFrame = { rect.x + 132, rect.y + 36, _titleWidth, _titleHeight };
        SDL_RenderCopy(SceneDirector::renderer, _titleTexture, NULL, &titleFrame);
    } else {
        // Title
        SDL_Rect titleFrame = { rect.x + 74, rect.y + 36, _titleWidth, _titleHeight };
        SDL_RenderCopy(SceneDirector::renderer, _titleTexture, NULL, &titleFrame);
    }

    // Divider
    AssetManager::setRenderColor(AssetManager::header_footer_divider);
    SDL_RenderDrawLine(SceneDirector::renderer, rect.x + 30, rect.y + 87, rect.w - 30, rect.y + 87);

    // Render any subviews
    View::render(rect, dTime);
}
