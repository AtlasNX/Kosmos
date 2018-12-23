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

#include <list>
#include <SDL2/SDL.h>

using namespace std;

class View {
    public:
        SDL_Rect frame;
        bool hidden;

        View();
        virtual ~View();

        virtual void render(SDL_Rect rect, double dTime);

        /* Controller Input */
        bool isFocusable;
        bool hasFocus;

        /* Touch Controls */
        bool isTouchable;
        virtual void touchStarted();
        virtual void touchMoved();
        virtual void touchEnded();

        /* View Hierarchy */
        View * superview;
        list<View *> subviews;
        void addSubView(View * view);
        void removeSubView(View * view);
};
