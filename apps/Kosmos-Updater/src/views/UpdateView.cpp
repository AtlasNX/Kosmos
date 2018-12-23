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

#include "UpdateView.hpp"
#include "../AssetManager.hpp"

UpdateView::UpdateView(string text) : View() {
    frame = { 0, 0, 1280, 325 };

    if (AssetManager::downloading == NULL) {
        AssetManager::downloading = AssetManager::loadAsset("downloading.png");
    }
    _downloadImageView = new ImageView(AssetManager::downloading);
    _downloadImageView->frame = { 400, 0, 479, 197 };

    _progressBarView = new ProgressBarView();
    _progressBarView->frame = { 437, 257, 411, 10 };

    _statusTextView = new TextView(AssetManager::subbody_font, text, AssetManager::text);
    _statusTextView->frame = { 0, 291, 1280, 0 };
    _statusTextView->textAlignment = CENTER_ALIGN;

    addSubView(_downloadImageView);
    addSubView(_progressBarView);
    addSubView(_statusTextView);
}

UpdateView::~UpdateView() {
    if (_downloadImageView == NULL)
        delete _downloadImageView;

    if (_progressBarView == NULL)
        delete _progressBarView;

    if (_statusTextView == NULL)
        delete _statusTextView;
}

void UpdateView::render(SDL_Rect rect, double dTime) {
    // Render any subviews.
    View::render(rect, dTime);
}

void UpdateView::setProgress(double progress) {
    _progressBarView->progress = progress;
}

void UpdateView::setText(string text) {
    _statusTextView->setText(text);
}
