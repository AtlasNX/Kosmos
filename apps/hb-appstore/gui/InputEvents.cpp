#include "InputEvents.hpp"

int TOTAL_BUTTONS = 17;

// computer key mappings
SDL_Keycode key_buttons[] = { SDLK_a, SDLK_b, SDLK_x, SDLK_y, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_RETURN, SDLK_l, SDLK_r, SDLK_z, SDLK_BACKSPACE, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT };

//SDL_GameControllerButton pad_buttons[] = { 0, 1, 2, 3, 13, 15, 12, 14, 10, 6, 7, 8, 11 };
SDL_GameControllerButton pad_buttons[] = { SDL_A, SDL_B, SDL_X, SDL_Y, SDL_UP, SDL_DOWN, SDL_LEFT, SDL_RIGHT, SDL_PLUS, SDL_L, SDL_R, SDL_ZL, SDL_MINUS, SDL_UP_STICK, SDL_DOWN_STICK, SDL_LEFT_STICK, SDL_RIGHT_STICK };

// our own "buttons" that correspond to the above SDL ones
unsigned int ie_buttons[] = { A_BUTTON, B_BUTTON, X_BUTTON, Y_BUTTON, UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON, START_BUTTON, L_BUTTON, R_BUTTON, Z_BUTTON, SELECT_BUTTON, UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON };

bool InputEvents::update()
{
  // get an event from SDL
  SDL_Event event;
  int ret = SDL_PollEvent(&event);
    
  // update our variables
  this->type = event.type;
  this->keyCode = -1;
  this->noop = false;
    
#ifdef PC
    this->allowTouch = false;
#endif
    
  if (event.key.repeat == 0 && (this->type == SDL_KEYDOWN || this->type == SDL_KEYUP))
  {
    this->keyCode = event.key.keysym.sym;
  }
  else if (this->type == SDL_JOYBUTTONDOWN || this->type == SDL_JOYBUTTONUP)
  {
    this->keyCode = event.jbutton.button;
  }
  else if (this->type == SDL_MOUSEMOTION || this->type == SDL_MOUSEBUTTONUP || this->type == SDL_MOUSEBUTTONDOWN)
  {
    bool isMotion = this->type == SDL_MOUSEMOTION;

    this->yPos = isMotion? event.motion.y : event.button.y;
    this->xPos = isMotion? event.motion.x : event.button.x;
  }
  else if (allowTouch && (this->type == SDL_FINGERMOTION || this->type == SDL_FINGERUP || this->type == SDL_FINGERDOWN))
  {
     this->yPos = event.tfinger.y * 720;
     this->xPos = event.tfinger.x * 1280;
  }
    
    toggleHeldButtons();

    // no more events to process
    if (ret == 0)
    {
        this->noop = processDirectionalButtons();
        return ret;
    }

    return true;
}

void InputEvents::toggleHeldButtons()
{
    int directionCode = directionForKeycode();
    
    if (directionCode >= 0)
    {
        if (isKeyDown())
        {
            // make sure it's not already down
            if (!held_directions[directionCode])
            {
                // on key down, set the corresponding held boolean to true
                held_directions[directionCode] = true;
                // reset the frame counter so we don't fire on this frame
                // (initial reset is lower to add a slight delay when they first start holding)
                curFrame = -25;
            }
        }
        
        if (isKeyUp())
        {
            // release the corresponding key too
            held_directions[directionCode] = false;
        }
    }
}

// returns true if a directional event was fire (so that we know to keep consuming later)
bool InputEvents::processDirectionalButtons()
{
    // up the counter
    curFrame ++;
    
    // if one of the four direction keys is true, fire off repeat events for it
    // (when rapidFire lines up only)
    if (curFrame > 0 && curFrame % rapidFireRate == 0)
    {
        for (int x=0; x<4; x++)
        {
            if (held_directions[x])
            {
                // send a corresponding directional event
                SDL_Event sdlevent;
                sdlevent.type = SDL_KEYDOWN;
                sdlevent.key.keysym.sym = key_buttons[4 + x];   // send up through right directions
                sdlevent.key.repeat = 0;
                SDL_PushEvent(&sdlevent);
                return false;    // only one direction at a time
            }
        }
    }
    
    return true;
}

int InputEvents::directionForKeycode()
{
    // returns 0 1 2 or 3 for up down left or right
    switch(this->keyCode)
    {
        case SDL_UP_STICK:
        case SDL_UP:
        case SDLK_UP:
            return 0;
        case SDL_DOWN_STICK:
        case SDL_DOWN:
        case SDLK_DOWN:
            return 1;
        case SDL_LEFT_STICK:
        case SDL_LEFT:
        case SDLK_LEFT:
            return 2;
        case SDL_RIGHT_STICK:
        case SDL_RIGHT:
        case SDLK_RIGHT:
            return 3;
        default:
            return -1;
    }
    return -1;
}

bool InputEvents::held(int buttons)
{
  // if it's a key event
  if (this->type == SDL_KEYDOWN || this->type == SDL_KEYUP)
  {
    for (int x=0; x<TOTAL_BUTTONS; x++)
      if (key_buttons[x] == keyCode && (buttons & ie_buttons[x]))
        return true;
  }

  // if it's a controller event
  else if (this->type == SDL_JOYBUTTONDOWN || this->type == SDL_JOYBUTTONUP)
  {
    for (int x=0; x<TOTAL_BUTTONS; x++)
      if (pad_buttons[x] == keyCode && (buttons & ie_buttons[x]))
        return true;
  }

  return false;
}

//bool InputEvents::held(int buttons)
//{
//    // held is the same as held internal, except it works with a timer
//    // will return true for 1. a press down event or 2. 128ms after that
//    // and then
//}

bool InputEvents::pressed(int buttons)
{
    return isKeyDown() && held(buttons);
}

bool InputEvents::released(int buttons)
{
    return isKeyUp() && held(buttons);
}

bool InputEvents::touchIn(int x, int y, int width, int height)
{
  return (this->xPos >= x &&
          this->xPos <= x+width &&
          this->yPos >= y &&
          this->yPos <= y+height);
}

bool InputEvents::isTouchDown()
{
    return this->type == SDL_MOUSEBUTTONDOWN || (allowTouch && this->type == SDL_FINGERDOWN);
}

bool InputEvents::isTouchDrag()
{
    return this->type == SDL_MOUSEMOTION || (allowTouch && this->type == SDL_FINGERMOTION);
}

bool InputEvents::isTouchUp()
{
    return this->type == SDL_MOUSEBUTTONUP || (allowTouch && this->type == SDL_FINGERUP);
}

bool InputEvents::isTouch()
{
    return isTouchDown() || isTouchDrag() || isTouchUp();
}

bool InputEvents::isKeyDown()
{
  return this->type == SDL_KEYDOWN || this->type == SDL_JOYBUTTONDOWN;
}

bool InputEvents::isKeyUp()
{
  return this->type == SDL_KEYUP || this->type == SDL_JOYBUTTONUP;
}
