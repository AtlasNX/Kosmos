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

#include <SDL2/SDL2_gfxPrimitives.h>
#include "ProgressBarView.hpp"
#include "../SceneDirector.hpp"

ProgressBarView::ProgressBarView() : View() {
    progress = 0;
}

ProgressBarView::~ProgressBarView() {}

void ProgressBarView::render(SDL_Rect rect, double dTime) {
    // Draw background.
    roundedBoxRGBA(
        SceneDirector::renderer,
        rect.x,
        rect.y,
        rect.x + rect.w,
        rect.y + rect.h, 
        rect.h / 2,
        AssetManager::disabled_text.r, AssetManager::disabled_text.g, AssetManager::disabled_text.b, AssetManager::disabled_text.a);

    // Draw progress bar.
    int progressWidth = (rect.w - rect.h) * progress;
    
    if (progressWidth > rect.w - rect.h)
        progressWidth = rect.w - rect.h;

    if (progressWidth < 0)
        progressWidth = 0;

    roundedBoxRGBA(
        SceneDirector::renderer,
        rect.x,
        rect.y,
        rect.x + rect.h + progressWidth,
        rect.y + rect.h, 
        rect.h / 2,
        AssetManager::active_text.r, AssetManager::active_text.g, AssetManager::active_text.b, AssetManager::active_text.a);

    // Render any subviews.
    View::render(rect, dTime);
}
