#include <string>
#include "Element.hpp"
#pragma once

#define NORMAL 0
#define MONOSPACED 1
#define ICON 2

class TextElement : public Element
{
public:
	TextElement(const char* text, int size, SDL_Color* color = 0, int font_type = NORMAL, int wrapped_width = 0);
	void render(Element* parent);
	SDL_Texture* renderText(std::string& message, int size, int font_type, int wrapped_width);

	SDL_Texture* textSurface = NULL;
	SDL_Color color;

    std::string* text;
private:
	int size;
};
