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

#include "ListRowView.hpp"
#include <SDL2/SDL2_gfxPrimitives.h>
#include "../SceneDirector.hpp"

ListRowView::ListRowView(string primaryText, string secondaryText, ListRowStyle style) : ControlView() {
    isLast = false;
    hasCheckmark = false;
    frame = { 0, 0, 850, 71 };

    _timeElapsed = 0;
    _isOn = false;
    _style = style;

    _primaryTextView = new TextView(AssetManager::body_font, primaryText, AssetManager::text);
    _primaryTextView->textAlignment = LEFT_ALIGN;
    if (style != SUBTITLE) {
        _primaryTextView->frame = { 21, 26, 808, 0 };
    } else {
        _primaryTextView->frame = { 21, 10, 808, 0 };
    }
    addSubView(_primaryTextView);

    if (style != DEFAULT) {
        if (style == BOOLEAN) {
            _secondaryTextView = new TextView(AssetManager::subbody_font, "Off", AssetManager::disabled_text);
        } else {
            _secondaryTextView = new TextView(AssetManager::subbody_font, secondaryText, (style == SUBTITLE) ? AssetManager::disabled_text : AssetManager::active_text);
        }

        if (style != SUBTITLE) {
            _secondaryTextView->textAlignment = RIGHT_ALIGN;
            _secondaryTextView->frame = { 21, 29, 808, 0 };
        } else {
            _secondaryTextView->textAlignment = LEFT_ALIGN;
            _secondaryTextView->frame = { 21, 41, 808, 0 };
        }

        addSubView(_secondaryTextView);
    } else {
        _secondaryTextView = NULL;
    }

    if (AssetManager::checkmark == NULL) {
        SDL_Surface *surface = TTF_RenderGlyph_Blended(AssetManager::button_font, 0xE14B, AssetManager::background);
        AssetManager::checkmark = SDL_CreateTextureFromSurface(SceneDirector::renderer, surface);
        SDL_FreeSurface(surface);
    }
}

ListRowView::~ListRowView() {
    if (_primaryTextView != NULL)
        delete _primaryTextView;

    if (_secondaryTextView != NULL)
        delete _secondaryTextView;
}

void ListRowView::render(SDL_Rect rect, double dTime) {
    // Draw Separators
    AssetManager::setRenderColor(AssetManager::list_divider);
    SDL_RenderDrawLine(SceneDirector::renderer, rect.x + 5, rect.y, rect.x + rect.w - 10, rect.y);
    if (isLast) {
        SDL_RenderDrawLine(SceneDirector::renderer, rect.x + 5, rect.y + rect.h, rect.x + rect.w - 10, rect.y + rect.h);
    }

    // Draw Focus Background
    if (hasFocus) {
        _timeElapsed += dTime;

        SDL_Rect backgroundFrame = { rect.x + 5, rect.y + 5, rect.w - 10, rect.h - 10 };
        AssetManager::setRenderColor(AssetManager::selected_background);
        SDL_RenderFillRect(SceneDirector::renderer, &backgroundFrame);

        SDL_Color selectionColor = _generateSelectionColor();
        _drawBorders(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, selectionColor);
    } else {
        _timeElapsed = 0;
    }

    if (hasCheckmark) {
        SDL_Color activeColor = AssetManager::active_text;
        filledCircleRGBA(SceneDirector::renderer, rect.x + rect.w - 35, rect.y + 36, 15, activeColor.r, activeColor.g, activeColor.b, activeColor.a);
        aacircleRGBA(SceneDirector::renderer, rect.x + rect.w - 35, rect.y + 36, 15, activeColor.r, activeColor.g, activeColor.b, activeColor.a);

        SDL_Rect checkmarkFrame = { rect.x + rect.w - 47, rect.y + 24, 25, 25 };
        SDL_RenderCopy(SceneDirector::renderer, AssetManager::checkmark, NULL, &checkmarkFrame);
    }

    // Render any subviews.
    ControlView::render(rect, dTime);
}

void ListRowView::setPrimaryText(string text) {
    _primaryTextView->setText(text);
}

void ListRowView::setSecondaryText(string text) {
    _secondaryTextView->setText(text);
}

void ListRowView::setIsOn(bool isOn) {
    _secondaryTextView->setText((isOn) ? "On" : "Off");
    _secondaryTextView->setTextColor((isOn) ? AssetManager::active_text : AssetManager::disabled_text);
}
