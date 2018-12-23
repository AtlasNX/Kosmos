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

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <switch.h>

using namespace std;

class AssetManager {
    public:
        /* Textures */
        static inline SDL_Texture * a_button = NULL;
        static inline SDL_Texture * b_button = NULL;
        static inline SDL_Texture * x_button = NULL;
        static inline SDL_Texture * y_button = NULL;
        static inline SDL_Texture * handheld = NULL;
        static inline SDL_Texture * checkmark = NULL;
        static inline SDL_Texture * downloading = NULL;
        static inline SDL_Texture * icon = NULL;

        /* Colors */
        static inline ColorSetId theme;
        static inline SDL_Color background;
        static inline SDL_Color sidebard_background;
        static inline SDL_Color header_footer_divider;
        static inline SDL_Color header_bullet;
        static inline SDL_Color list_divider;
        static inline SDL_Color active_player_indicator;
        static inline SDL_Color player_indicator;
        static inline SDL_Color selected_background;
        static inline SDL_Color selected_border_1;
        static inline SDL_Color selected_border_2;
        static inline SDL_Color modal_faded_background;
        static inline SDL_Color modal_background;
        static inline SDL_Color text;
        static inline SDL_Color active_text;
        static inline SDL_Color disabled_text;

        /* Fonts */
        static inline PlFontData standardFontData;
        static inline TTF_Font * header_font = NULL;
        static inline TTF_Font * body_font = NULL;
        static inline TTF_Font * subbody_font = NULL;
        static inline PlFontData extendedFontData;
        static inline TTF_Font * button_font = NULL;
        static inline TTF_Font * large_button_font = NULL;
 
        /* Sounds */
        static inline Mix_Chunk * enter = NULL;
        static inline Mix_Chunk * back = NULL;
        static inline Mix_Chunk * select = NULL;
        
        static bool initialize();
        static void dealloc();
        static void setRenderColor(SDL_Color color);
        static SDL_Texture * loadAsset(string file);
};
