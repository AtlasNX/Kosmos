#include "ListElement.hpp"
#include <SDL2/SDL2_gfxPrimitives.h>

bool ListElement::process(InputEvents* event)
{
    bool ret = false;
    
    // perform inertia scrolling for this element
    ret |= this->handleInertiaScroll(event);
    
    ret |= super::process(event);
    
    return ret;
}

bool ListElement::handleInertiaScroll(InputEvents* event)
{
    bool ret = false;
    ListElement* elem = this;
    
  if (event->isTouchDown())
  {
    // make sure that the mouse down's X coordinate is over the app list (not sidebar)
    if (event->xPos < elem->x)
      return false;

    // saw mouse down so set it in our element object
    elem->dragging = true;
    elem->lastMouseY = event->yPos;
      elem->initialTouchDown = event->yPos;
      
      ret |= true;
  }
  // drag event for scrolling up or down
  else if (event->isTouchDrag())
  {
    if (elem->dragging)
    {
        // prevent scrolling until we exceed a treshold distance in the Y direction
        if (this->initialTouchDown >= 0 && (abs(event->yPos - this->initialTouchDown) < 10))
            return false;

        this->initialTouchDown = -1;
        
      int distance = event->yPos - elem->lastMouseY;
      elem->y += distance;
      elem->lastMouseY = event->yPos;

      // use the last distance as the rubber band value
      elem->elasticCounter = distance;
        
      ret |= true;
    }
  }
  else if (event->isTouchUp())
  {
    // mouse up, no more mouse down (TODO: fire selected event here)
    elem->dragging = false;

    // if the scroll offset is less than the total number of apps
    // (put on the mouse up to make it "snap" when going out of bounds)
    // TODO: account for max number of apps too (prevent scrolling down forever)
    if (elem->y > 0)
      elem->y = 0;
      
    ret |= true;
  }

  // if mouse is up, and there's some elastic counter left, burn out remaining elastic value
  if (!elem->dragging && elem->elasticCounter != 0)
  {
    elem->y += elem->elasticCounter;

    int positivity = elem->elasticCounter/abs(elem->elasticCounter);
    elem->elasticCounter += 10 * (-1 * positivity);

    // when the oval and the elastic counter don't match in positivity, reset it to 0
    if (elem->elasticCounter != 0 && elem->elasticCounter/abs(elem->elasticCounter) != positivity)
      elem->elasticCounter = 0;

    // TODO: same problem as above todo, also extract into method?
    if (elem->y > 0)
      elem->y = 0;
    
      ret |= true;
  }
    
    return ret;
}
