#include "Element.hpp"
#include "TextElement.hpp"
#include "ImageElement.hpp"
#include "AppList.hpp"
#include "ImageCache.hpp"
#include <unordered_map>

class MainDisplay : public Element
{
public:
	MainDisplay(Get* get);
	bool process(InputEvents* event);
	void render(Element* parent);
	void background(int r, int g, int b);
	void update();
    
    TextElement* notice = NULL;

	static SDL_Renderer* mainRenderer;
    static Element* subscreen;
    static MainDisplay* mainDisplay;

	Get* get = NULL;
	ImageCache* imageCache = NULL;
    bool error = false;
    
    int lastFrameTime = 99;
    SDL_Event needsRender;

	bool showingSplash = true;
	int count = 0;
};

void quit();
