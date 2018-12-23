#include "Element.hpp"
#include "TextElement.hpp"
#include <SDL2/SDL_image.h>
#pragma once

// bad idea, TODO: Input-able interface for elements that can be typed-in
class Feedback;
class AppList;

class Keyboard : public Element
{
public:
    Keyboard(AppList* appList, std::string* myText, Feedback* feedback = NULL);
    ~Keyboard();
	void render(Element* parent);
    bool process(InputEvents* event);
    
    // setup field variables
    void updateSize();
    
	// draw a qwerty keyboard
	std::string row1 = "Q W E R T Y U I O P";
	std::string row2 =  "A S D F G H J K L";
	std::string row3 =   "Z X C V B N M";

	std::vector<std::string> rows;
    
    // the pointer to the text for this keyboard to modify
    std::string* myText;
    
    // the currently selected row and index
    int curRow = -1;
    int index = -1;
    
    // the below variables are stored to be used in processing touch events
    // and rendering the drawings to screen
    
    // attributes of each key
    int keyWidth = 0;
    int padding = 0;
    int textSize = 0;
    
    // attributes of delete and backspace keys
    int dPos = 0;
    int dHeight = 0;
    int sPos = 0;
    int dWidth = 0;
    int sWidth = 0;
    
    // positions of key location offset information
    int kXPad = 0;
    int kXOff = 0;
    int yYOff = 0;
    int kYPad = 0;
    int ySpacing = 0;
    
    bool touchMode = true;

	AppList* appList = NULL;
    Feedback* feedback = NULL;
    
    void space();
    void backspace();
    void type(int y, int x);
    void updateView();
};
