#include "Element.hpp"
#pragma once

// ListElement keeps track of a currently highlighted piece, as well as handling scrolling as a unit
class ListElement : public Element
{
public:
    int highlighted = -1;
    int initialTouchDown = -1;
    bool process(InputEvents* event);
	bool handleInertiaScroll(InputEvents* event);
};
