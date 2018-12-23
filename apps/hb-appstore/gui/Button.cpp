#include "Button.hpp"

Button::Button(const char* message, int button, bool dark, int size, int width)
{
    int PADDING = 10;
    SDL_Color color;

    if (dark)
        color = {0xff, 0xff, 0xff, 0xff};
    else
        color = {0x00, 0x00, 0x00, 0xff};

    this->dark = dark;

    const char* unicode;

    switch (button)
    {
        case A_BUTTON:
            unicode = "\ue0a0";
            break;
        case B_BUTTON:
            unicode = "\ue0a1";
            break;
        case Y_BUTTON:
            unicode = "\ue0a2";
            break;
        case X_BUTTON:
            unicode = "\ue0a3";
            break;
        default:
            unicode = "";
    }

    this->physical = button;

    TextElement* icon = new TextElement(unicode, size*1.25, &color, ICON);
    this->elements.push_back(icon);
    icon->position(PADDING*2, PADDING);

    TextElement* text = new TextElement(message, size, &color);
//    icon->resize(text->height, text->height);

    int bWidth = PADDING*0.5*(icon->width!=0);  // gap space between button

    text->position(PADDING*2 + bWidth + icon->width, PADDING);
    this->elements.push_back(text);

    this->width = (width > 0)? width : text->width + PADDING*4 + bWidth + icon->width;
    this->height = text->height + PADDING*2;

    icon->position(PADDING*1.7, PADDING + (text->height - icon->height)/2);

    this->touchable = true;

    // TODO: add icon and make room for it in the x, y dimens
}

void Button::position(int x, int y)
{
    ox = x;
    oy = y;
    super::position(x, y);
}

bool Button::process(InputEvents* event)
{
  if (event->isKeyDown() && event->held(this->physical))
  {
    // invoke our action, since we saw a physical button press that matches!
    this->action();
    return true;
  }

  return super::process(event);
}

void Button::render(Element* parent)
{
    if (this->parent == NULL)
        this->parent = parent;

    this->renderer = parent->renderer;

    // update our x and y according to our parent
    this->x = ox + parent->x;
    this->y = oy + parent->y;

    // draw bg for button
    SDL_Rect dimens = { x, y, width, height };

    if (dark)
    {
        SDL_SetRenderDrawColor(parent->renderer, 0x67, 0x6a, 0x6d, 0xFF);
#if defined(__WIIU__)
        SDL_SetRenderDrawColor(parent->renderer, 0x3b, 0x3c, 0x4e, 0xFF);
#endif
    }
    else
        SDL_SetRenderDrawColor(parent->renderer, 0xee, 0xee, 0xee, 0xFF);

    SDL_RenderFillRect(parent->renderer, &dimens);

    super::render(this);
}
