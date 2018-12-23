#include "MainDisplay.hpp"

TextElement::TextElement(const char* text, int size, SDL_Color* color, int font_type, int wrapped_width)
{
	this->text = new std::string(text);
	this->size = size;

	if (color == NULL)
		this->color = {0xff, 0xff, 0xff};
	else
		this->color = *color;

	this->textSurface = this->renderText(*(this->text), size, font_type, wrapped_width);
    
    int w, h;
    SDL_QueryTexture(this->textSurface, NULL, NULL, &w, &h);
    this->width = w;
    this->height = h;
}

void TextElement::render(Element* parent)
{
	if (this->hidden)
		return;

	SDL_Rect textLocation;
	textLocation.x = this->x + parent->x;
	textLocation.y = this->y + parent->y;
	textLocation.w = this->width;
	textLocation.h = this->height;

	// std::cout << this->text->c_str() << " [" << this->x << " " << parent->x << " " <<
	// this->y << " " << parent->y << " " <<
	// textLocation.w << " " << textLocation.h << std::endl;

	SDL_RenderCopy(MainDisplay::mainRenderer, this->textSurface, NULL, &textLocation);
}

SDL_Texture* TextElement::renderText(std::string& message, int size, int font_type, int wrapped_width)
{
	std::string key = message + std::to_string(size);

	// try to find it in the cache first
    if (ImageCache::cache.count(key))
         return ImageCache::cache[key];

	// not found, make/render it

	TTF_Font* font;

	if (font_type == MONOSPACED)
		font = TTF_OpenFont(ROMFS "./res/mono.ttf", size);
    else if (font_type == ICON)
        font = TTF_OpenFont(ROMFS "./res/nxicons.ttf", size);
	else
		font = TTF_OpenFont(ROMFS "./res/opensans.ttf", size);

	// font couldn't load, don't render anything
	if (!font)
		return NULL;

	SDL_Surface* surf;
    if (font_type == ICON)
        surf = TTF_RenderUTF8_Blended(font, message.c_str(), this->color);
	else if (wrapped_width == 0)
		surf = TTF_RenderText_Blended(font, message.c_str(), this->color);
	else
		surf = TTF_RenderText_Blended_Wrapped(font, message.c_str(), this->color, wrapped_width);

	SDL_Texture* texture = SDL_CreateTextureFromSurface(MainDisplay::mainRenderer, surf);
    SDL_FreeSurface(surf);

	//	SDL_FreeSurface(surf);
	TTF_CloseFont(font);

	// save it to the cache for later
	ImageCache::cache[key] = texture;

	return texture;
}
