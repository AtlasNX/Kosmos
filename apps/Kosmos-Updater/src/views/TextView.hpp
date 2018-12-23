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

#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include "../View.hpp"

typedef enum {
    LEFT_ALIGN,
    CENTER_ALIGN,
    RIGHT_ALIGN
} TextAlignment;

using namespace std;

class TextLine {
    public:
        SDL_Texture * textTexture;
        int textWidth;
        int textHeight;
};

class TextView : public View {
    public:
        TTF_Font * font;
        string text;
        SDL_Color textColor;
        TextAlignment textAlignment;
        int alpha;
        int lineHeight;

        TextView(TTF_Font * theFont, string theText, SDL_Color theTextColor);
        ~TextView();

        void render(SDL_Rect rect, double dTime);
        int getTextHeight();
        void setFont(TTF_Font * theFont);
        void setText(string theText);
        void setTextColor(SDL_Color theTextColor);
        void setLineHeight(int lineHeight);

    private:
        vector<TextLine *> _textLines;
        void _reset();
};
