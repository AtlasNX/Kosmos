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

#include <switch.h>
#include "FooterView.hpp"
#include "../AssetManager.hpp"
#include "../SceneDirector.hpp"

FooterView::FooterView() : View() {
    isFocusable = false;
    isTouchable = false;

    if (AssetManager::a_button == NULL) {
        SDL_Surface *surface = TTF_RenderGlyph_Blended(AssetManager::button_font, 0xE0E0, AssetManager::text);
        AssetManager::a_button = SDL_CreateTextureFromSurface(SceneDirector::renderer, surface);
        SDL_FreeSurface(surface);
    }

    if (AssetManager::b_button == NULL) {
        SDL_Surface *surface = TTF_RenderGlyph_Blended(AssetManager::button_font, 0xE0E1, AssetManager::text);
        AssetManager::b_button = SDL_CreateTextureFromSurface(SceneDirector::renderer, surface);
        SDL_FreeSurface(surface);
    }

    if (AssetManager::x_button == NULL) {
        SDL_Surface *surface = TTF_RenderGlyph_Blended(AssetManager::button_font, 0xE0E2, AssetManager::text);
        AssetManager::x_button = SDL_CreateTextureFromSurface(SceneDirector::renderer, surface);
        SDL_FreeSurface(surface);
    }

    if (AssetManager::y_button == NULL) {
        SDL_Surface *surface = TTF_RenderGlyph_Blended(AssetManager::button_font, 0xE0E3, AssetManager::text);
        AssetManager::y_button = SDL_CreateTextureFromSurface(SceneDirector::renderer, surface);
        SDL_FreeSurface(surface);
    }

    if (AssetManager::handheld == NULL) {
        SDL_Surface *surface = TTF_RenderGlyph_Blended(AssetManager::large_button_font, 0xE121, AssetManager::text);
        AssetManager::handheld = SDL_CreateTextureFromSurface(SceneDirector::renderer, surface);
        SDL_FreeSurface(surface);
    }
}

FooterView::~FooterView() {
    for (auto const& action : actions) {
        delete action;
    }

    actions.clear();
}

void FooterView::render(SDL_Rect rect, double dTime) {
    // Draw Background
    AssetManager::setRenderColor(AssetManager::background);
    SDL_RenderFillRect(SceneDirector::renderer, &rect);

    // Divider.
    AssetManager::setRenderColor(AssetManager::header_footer_divider);
    SDL_RenderDrawLine(SceneDirector::renderer, rect.x + 30, rect.y, rect.w - 30, rect.y);

    SDL_Rect handheldFrame = { 55, rect.y + 2, 70, 70 };
    SDL_RenderCopy(SceneDirector::renderer, AssetManager::handheld, NULL, &handheldFrame);

    // Render Actions.
    int current_x = rect.w - 60;
    for (auto const& action : actions) {
        // Create texture if it doesn't already exists.
        if (action->textTexture == NULL) {
            SDL_Surface *surface = TTF_RenderText_Blended(AssetManager::body_font, action->text.c_str(), AssetManager::text);
            action->textTexture = SDL_CreateTextureFromSurface(SceneDirector::renderer, surface);
            action->textWidth = surface->w;
            action->textHeight = surface->h;
            SDL_FreeSurface(surface);
        }

        // Render Action Text
        current_x -= action->textWidth;
        SDL_Rect textFrame = { current_x, rect.y + 25, action->textWidth, action->textHeight };
        SDL_RenderCopy(SceneDirector::renderer, action->textTexture, NULL, &textFrame);

        current_x -= 37;
        // Render Action Button Icon
        SDL_Rect iconFrame = { current_x, rect.y + 25, 25, 25 };
        switch(action->button) {
            case B_BUTTON:
                SDL_RenderCopy(SceneDirector::renderer, AssetManager::b_button, NULL, &iconFrame);
                break;

            case X_BUTTON:
                SDL_RenderCopy(SceneDirector::renderer, AssetManager::x_button, NULL, &iconFrame);
                break;

            case Y_BUTTON:
                SDL_RenderCopy(SceneDirector::renderer, AssetManager::y_button, NULL, &iconFrame);
                break;

            default:
                SDL_RenderCopy(SceneDirector::renderer, AssetManager::a_button, NULL, &iconFrame);
                break;
        }
        current_x -= 40;
    }

    // Render any subviews.
    View::render(rect, dTime);
}
