#include "TextElement.hpp"
#include "ImageElement.hpp"

class Button : public Element
{
public:
    Button(const char* text, int button, bool dark = false, int size = 20, int width = 0);
    bool process(InputEvents* event);
    void render(Element* parent);
    void position(int x, int y);

    // original x and y coordinates of this button before add in the parent
    int ox = 0, oy = 0;

    // the physical button to activate this button
    int physical = -1;

    bool dark = false;


    // TOOD: callback -> part of Element?
};
