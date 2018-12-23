#include <SDL2/SDL.h>

class Console
{
public:
	// console methods
	Console();
	void background(int r, int g, int b);
	void putAPixel(int x, int y, int r, int g, int b);
	void fillRect(int x, int y, int width, int height, int r, int g, int b);
	void drawString(int xi, int yi, const char* string);
	void drawColorString(int xi, int yi, const char* string, int r, int g, int b);
	void close();
	void update();

private:

	// SDL graphics variables
	SDL_Surface* window_surface;
	SDL_Window* window;
};

void sleep(int s);
char* fontLookup(char c);
