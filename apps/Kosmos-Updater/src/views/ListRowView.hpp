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

#include <string>
#include "ControlView.hpp"
#include "TextView.hpp"

typedef enum {
    DEFAULT,
    SUBTITLE,
    VALUE,
    BOOLEAN
} ListRowStyle;

using namespace std;

class ListRowView : public ControlView {
    public:
        bool isLast;
        bool hasCheckmark;
        
        ListRowView(string primaryText, string secondaryText, ListRowStyle style);
        ~ListRowView();

        void render(SDL_Rect rect, double dTime);

        void setPrimaryText(string text);
        void setSecondaryText(string text);
        void setIsOn(bool isOn);
    private:
        bool _isOn;

        TextView * _primaryTextView;
        TextView * _secondaryTextView;

        ListRowStyle _style;

        void _renderDefaultStyle(SDL_Rect rect);
        void _renderSubtitleStyle(SDL_Rect rect);
        void _renderValueStyle(SDL_Rect rect);
};
