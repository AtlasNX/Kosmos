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

#include "ControlView.hpp"
#include <SDL2/SDL2_gfxPrimitives.h>
#include "../AssetManager.hpp"
#include "../SceneDirector.hpp"

SDL_Color ControlView::_generateSelectionColor() {
    SDL_Color color1 = AssetManager::selected_border_1;
    SDL_Color color2 = AssetManager::selected_border_2;
    SDL_Color result = { 0, 0, 0, 255 };

    
    if (_timeElapsed > 2000)
        _timeElapsed -= 2000;

    // Color1 -> Color2
    if (floor(((int) _timeElapsed) / 1000) == 0) {
        double time = (_timeElapsed / 1000) * -1;
        result.r = color1.r + floor((color1.r - color2.r) * time);
        result.g = color1.g + floor((color1.g - color2.g) * time);
        result.b = color1.b + floor((color1.b - color2.b) * time);
    }
    // Color2 -> Color1
    else {
        double time = ((_timeElapsed - 1000) / 1000) * -1;
        result.r = color2.r + floor((color2.r - color1.r) * time);
        result.g = color2.g + floor((color2.g - color1.g) * time);
        result.b = color2.b + floor((color2.b - color1.b) * time);
    }

    return result;
}

void ControlView::_drawBorders(int x1, int y1, int x2, int y2, SDL_Color color) {
    // Top
    thickLineRGBA(
            SceneDirector::renderer,
            x1, y1 + 3, x2, y1 + 3,
            5,
            color.r, color.g, color.b, color.a);

    // Right
    thickLineRGBA(
            SceneDirector::renderer,
            x2 - 2, y1 + 3, x2 - 2, y2 - 3,
            5,
            color.r, color.g, color.b, color.a);

    // Bottom
    thickLineRGBA(
            SceneDirector::renderer,
            x2, y2 - 3, x1, y2 - 3,
            5,
            color.r, color.g, color.b, color.a);

    // Left
    thickLineRGBA(
            SceneDirector::renderer,
            x1 + 2, y2 - 3, x1 + 2, y1 + 3,
            5,
            color.r, color.g, color.b, color.a);
}