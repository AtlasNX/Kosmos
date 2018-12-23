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

#include "TextView.hpp"
#include <sstream>
#include "../SceneDirector.hpp"

TextView::TextView(TTF_Font * theFont, string theText, SDL_Color theTextColor) : View() {
    isFocusable = false;
    isTouchable = false;
    textAlignment = LEFT_ALIGN;
    alpha = 255;
    lineHeight = TTF_FontHeight(theFont);
    
    font = theFont;
    text = theText;
    textColor = theTextColor;
    _reset();
}

TextView::~TextView() {
    for (auto const& textLine : _textLines) {
        SDL_DestroyTexture(textLine->textTexture);
    }
    _textLines.clear();
}

void TextView::render(SDL_Rect rect, double dTime) {
    int y = rect.y;
    for (auto const& textLine : _textLines) {
        SDL_SetTextureAlphaMod(textLine->textTexture, alpha);

        int x = 0;
        int width = max(textLine->textWidth, rect.w);
        switch (textAlignment) {
            case LEFT_ALIGN:
                x = rect.x;
                break;

            case CENTER_ALIGN:
                x = rect.x + (width - textLine->textWidth) / 2;
                break;

            case RIGHT_ALIGN:
                x = rect.x + width - textLine->textWidth;
                break;
        }

        SDL_Rect textFrame = { x, y, textLine->textWidth, textLine->textHeight };
        SDL_RenderCopy(SceneDirector::renderer, textLine->textTexture, NULL, &textFrame);

        y += lineHeight;
    }

    // Render any subviews.
    View::render(rect, dTime);
}

int TextView::getTextHeight() {
    return _textLines.size() * lineHeight;
}

void TextView::setFont(TTF_Font * theFont) {
    font = theFont;
    _reset();
}

void TextView::setText(string theText) {
    text = theText;
    _reset();
}

void TextView::setTextColor(SDL_Color theTextColor) {
    textColor = theTextColor;
    _reset();
}

void TextView::_reset() {
    for (auto const& textLine : _textLines) {
        SDL_DestroyTexture(textLine->textTexture);
    }
    _textLines.clear();

    stringstream textStream = stringstream(text);
    string text;
    while(getline(textStream, text, '\n')) {
        SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), textColor);
        if (surface != NULL) {
            TextLine * textLine = new TextLine();
            textLine->textTexture = SDL_CreateTextureFromSurface(SceneDirector::renderer, surface);
            textLine->textWidth = surface->w;
            textLine->textHeight = surface->h;
            SDL_FreeSurface(surface);
            _textLines.push_back(textLine);
        }
    }
}
