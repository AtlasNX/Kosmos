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

#include "Scene.hpp"

Scene::Scene() {
    _touchedView = NULL;
}

Scene::~Scene() {}

void Scene::handleButton(u32 buttons) {}

void Scene::render(SDL_Rect rect, double dTime) {
    for (auto const& view : subviews) {
        if (!view->hidden) {
            SDL_Rect subviewFrame = view->frame;
            view->render({ rect.x + subviewFrame.x, rect.y + subviewFrame.y, subviewFrame.w, subviewFrame.h }, dTime);
        }
    }
}

void Scene::touchStarted() {
    for (auto const& view : subviews) {
        // TODO: Check if touch is within the view
        if (view->isTouchable) {
            _touchedView = view;
            _touchedView->touchStarted();
        }
    }
}

void Scene::touchMoved() {
    if (_touchedView != NULL) {
        _touchedView->touchMoved();
    }
}

void Scene::touchEnded() {
    if (_touchedView != NULL) {
        _touchedView->touchEnded();
    }
}

void Scene::addSubView(View * view) {
    view->superview = NULL;
    subviews.push_back(view);
}

void Scene::removeSubView(View * view) {
    view->superview = NULL;
    subviews.remove(view);
}
