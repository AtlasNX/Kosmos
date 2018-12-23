// "PAD" controls (unique to us)
#define BUTTON_LEFT		0b00000001
#define BUTTON_RIGHT	0b00000010
#define BUTTON_UP		0b00000100
#define BUTTON_DOWN		0b00001000

#define BUTTON_X		0b00010000
#define BUTTON_MINUS	0b00100000

#define BUTTON_A		0b01000000
#define BUTTON_B		0b10000000

// switch controls
#define JOYPAD_LEFT 	0x1000
#define JOYPAD_DOWN 	0x8000
#define JOYPAD_RIGHT	0x4000
#define JOYPAD_UP		0x2000
#define JOYPAD_START	0x0400
#define JOYPAD_SELECT	0x0800
#define JOYPAD_X		0x0004
#define JOYPAD_Y		0x0008
#define JOYPAD_B		0x0002
#define JOYPAD_A		0x0001
#define JOYPAD_R		0x0080
#define JOYPAD_L		0x0040
#define JOYPAD_R2		0x0200
#define JOYPAD_L2		0x0100


#include <SDL2/SDL.h>

class Input
{
public:
	Input();
	void close();
	void updateButtons();
	bool held(char b);
	char btns_h;

private:
	float lstick_x;
	float lstick_y;
	float rstick_x;
	float rstick_y;
};
