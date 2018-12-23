#include "Element.hpp"
#include <SDL2/SDL2_gfxPrimitives.h>

bool Element::process(InputEvents* event)
{
    // whether or not we need to update the screen
    bool ret = false;
    
    // do any touch down, drag, or up events
    if (touchable)
    {
        ret |= onTouchDown(event);
        ret |= onTouchDrag(event);
        ret |= onTouchUp(event);
    }
    
	// call process on subelements
	for (int x=0; x<this->elements.size(); x++)
        if (this->elements.size() > x && this->elements[x])
            ret |= this->elements[x]->process(event);
    
    ret |= this->needsRedraw;
    this->needsRedraw = false;
    
	return ret;
}

void Element::render(Element* parent)
{
	for (int x=0; x<this->elements.size(); x++)
	{
		// go through every subelement and run render
		this->elements[x]->render(parent);
	}
    
    // if we're touchable, and we have some animation counter left, draw a rectangle+overlay
    if (this->touchable && this->elasticCounter > THICK_HIGHLIGHT)
    {
        SDL_Rect d = { this->xOff + this->x - 5, this->yOff + this->y - 5, this->width + 10, this->height + 10};
        SDL_SetRenderDrawBlendMode(parent->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(parent->renderer, 0xad, 0xd8, 0xe6, 0x90);
        SDL_RenderFillRect(parent->renderer, &d);
    }
    
    if (this->touchable && this->elasticCounter > NO_HIGHLIGHT)
    {
        SDL_Rect d = { this->xOff + this->x - 5, this->yOff + this->y - 5, this->width + 10, this->height + 10};
        rectangleRGBA(parent->renderer, d.x, d.y, d.x + d.w, d.y + d.h, 0x66, 0x7c, 0x89, 0xFF);
        
        if (this->elasticCounter == THICK_HIGHLIGHT)
        {
            // make it a little thicker by drawing more rectangles TODO: better way to do this?
            for (int x=0; x<5; x++)
            {
                rectangleRGBA(parent->renderer, d.x + x, d.y + x, d.x + d.w - x, d.y + d.h - x, 0x66 - x*10, 0x7c + x*20, 0x89 + x*10, 0xFF);
            }
        }
    }
}

void Element::position(int x, int y)
{
	this->x = x;
	this->y = y;
}

bool Element::onTouchDown(InputEvents* event)
{
    if (!event->isTouchDown())
        return false;
    
    if (!event->touchIn(this->xOff + this->x, this->yOff + this->y, this->width, this->height))
        return false;
    
    // mouse pushed down, set variable
    this->dragging = true;
    this->lastMouseY = event->yPos;
    this->lastMouseX = event->xPos;
    
    // turn on deep highlighting during a touch down
    if (this->touchable)
        this->elasticCounter = DEEP_HIGHLIGHT;
    
    return true;
}

bool Element::onTouchDrag(InputEvents* event)
{
    bool ret = false;
    
    if (!event->isTouchDrag())
        return false;
    
    // minimum amount of wiggle allowed by finger before calling off a touch event
    int TRESHOLD = 40;
    
    // we've dragged out of the icon, invalidate the click by invoking onTouchUp early
    // check if we haven't drifted too far from the starting variable (treshold: 40)
    if (this->dragging && (abs(event->yPos - this->lastMouseY) >= TRESHOLD || abs(event->xPos - this->lastMouseX) >= TRESHOLD))
    {
        ret |= (this->elasticCounter > 0);
        this->elasticCounter = NO_HIGHLIGHT;
    }
    
    // ontouchdrag never decides whether to update the view or not
    return ret;
}

bool Element::onTouchUp(InputEvents* event)
{
    if (!event->isTouchUp())
        return false;
    
    bool ret;
    
    // ensure we were dragging first (originally checked the treshold above here, but now that actively invalidates it)
    if (this->dragging)
    {
        // check that this click is in the right coordinates for this square
        // and that a subscreen isn't already being shown
        // TODO: allow buttons to activae this too?
        if (event->touchIn(this->xOff + this->x, this->yOff + this->y, this->width, this->height))
        {
            // elasticCounter must be nonzero to allow a click through (highlight must be shown)
            if (this->elasticCounter > 0 && action != NULL)
            {
                // invoke this element's action
                this->action();
                ret |= true;
            }
        }
    }
    
    // release mouse
    this->dragging = false;
    
    // update if we were previously highlighted, cause we're about to remove it
    ret |= (this->elasticCounter > 0);
    
    this->elasticCounter = 0;
    
    return ret;
}

void Element::hide()
{
	this->hidden = true;
}

void Element::wipeElements()
{
	for (int x=0; x<this->elements.size(); x++)
	{
		Element* elem = this->elements[x];
		delete elem;
	}

	this->elements.clear();
}
