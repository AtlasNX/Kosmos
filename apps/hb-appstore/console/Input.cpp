#include "Input.hpp"

Input::Input()
{
#if defined(INPUT_JOYSTICK)
	if(SDL_Init(SDL_INIT_JOYSTICK) < 0)
		return;

	for (int i = 0; i < SDL_NumJoysticks(); i++)
		if (!SDL_JoystickOpen(i))
			printf("SDL_JoystickOpen: %s\n", SDL_GetError());
#endif
}

void Input::close()
{
}

void Input::updateButtons()
{
	SDL_Event event;
	SDL_PollEvent(&event);

	// reset buttons
	this->btns_h = 0b00000000;

	this->lstick_x = 0;
	this->lstick_y = 0;
	this->rstick_x = 0;
	this->rstick_x = 0;

#if defined(INPUT_JOYSTICK)
	if (event.type == SDL_JOYBUTTONDOWN)
	{
		this->btns_h |= ((event.jbutton.button == SDL_CONTROLLER_BUTTON_A)?				BUTTON_A		: 0);
		this->btns_h |= ((event.jbutton.button == SDL_CONTROLLER_BUTTON_B)?				BUTTON_B		: 0);
		this->btns_h |= ((event.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)?		BUTTON_UP		: 0);
		this->btns_h |= ((event.jbutton.button == SDL_CONTROLLER_BUTTON_MAX)?			BUTTON_DOWN		: 0);
		this->btns_h |= ((event.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)?		BUTTON_LEFT		: 0);
		this->btns_h |= ((event.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)?	BUTTON_RIGHT 	: 0);
		this->btns_h |= ((event.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)?		BUTTON_MINUS	: 0);
	}
#else
	if (event.type == SDL_KEYDOWN)
	{
		this->btns_h |= ((event.key.keysym.sym == SDLK_a)?			BUTTON_A : 0);
		this->btns_h |= ((event.key.keysym.sym == SDLK_b)?			BUTTON_B : 0);
		this->btns_h |= ((event.key.keysym.sym == SDLK_UP)?			BUTTON_UP : 0);
		this->btns_h |= ((event.key.keysym.sym == SDLK_DOWN)?		BUTTON_DOWN : 0);
		this->btns_h |= ((event.key.keysym.sym == SDLK_LEFT)?		BUTTON_LEFT : 0);
		this->btns_h |= ((event.key.keysym.sym == SDLK_RIGHT)?		BUTTON_RIGHT : 0);
		this->btns_h |= ((event.key.keysym.sym == SDLK_MINUS|| event.key.keysym.sym == SDLK_RETURN)?		BUTTON_MINUS : 0);
		this->btns_h |= ((event.key.keysym.sym == SDLK_x)?			BUTTON_X : 0);
	}
#endif
}

bool Input::held(char b)
{
	return this->btns_h & b;
}
