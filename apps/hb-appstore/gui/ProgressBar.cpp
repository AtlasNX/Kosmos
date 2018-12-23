#include "ProgressBar.hpp"
#include <SDL2/SDL2_gfxPrimitives.h>

ProgressBar::ProgressBar()
{
	// total width of full progress bar
	this->width = 450;
	this->color = 0x56c1dfff;
}

void ProgressBar::render(Element* parent)
{
    if (this->percent < 0)
        return;
    
    if (dimBg)
    {
        // draw a big dim layer around the entire window before drawing this progress bar
        SDL_Rect dim = { 0, 0, 1280, 720 };
        
        SDL_SetRenderDrawBlendMode(parent->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(parent->renderer, 0x00, 0x00, 0x00, 0xbb);
        SDL_RenderFillRect(parent->renderer, &dim);
    }
    
	SDL_Rect location;
	int x = this->x + parent->x;
	int y = this->y + parent->y;

	int blue = this->color;
//	int gray = 0x989898ff;
    
    

	// draw full grayed out bar first
	SDL_Rect gray_rect;
	gray_rect.x = x;
	gray_rect.y = y-4;
	gray_rect.w = width;
	gray_rect.h = 9;

	SDL_SetRenderDrawColor(parent->renderer, 0x98, 0x98, 0x98, 0xff);	//gray2
	SDL_RenderFillRect(parent->renderer, &gray_rect);

	// draw ending "circle"
	filledCircleRGBA(parent->renderer, x + this->width, y, 5, 0x98, 0x98, 0x98, 0xff);
	
	// draw left "circle" (rounded part of bar)
	filledCircleRGBA(parent->renderer, x, y, 5, 0x56, 0xc1, 0xdf, 0xff);

	// draw blue progress bar so far
	SDL_Rect blue_rect;
	blue_rect.x = x;
	blue_rect.y = y-4;
	blue_rect.w = width*this->percent;
	blue_rect.h = 9;

	SDL_SetRenderDrawColor(parent->renderer, 0x56, 0xc1, 0xdf, 0xff);	// blue2
	SDL_RenderFillRect(parent->renderer, &blue_rect);

	// draw right "circle" (rounded part of bar, and ending)
	filledCircleRGBA(parent->renderer, x + width*this->percent, y, 5, 0x56, 0xc1, 0xdf, 0xff);

}
