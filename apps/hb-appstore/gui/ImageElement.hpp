#include "Element.hpp"
#include <SDL2/SDL_image.h>
#pragma once

class ImageElement : public Element
{
public:
	ImageElement(const char* path);
	void render(Element* parent);

	SDL_Texture* imgSurface = NULL;
	const char* path;

	void resize(int width, int height);
};
