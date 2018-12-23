#include "Menu.hpp"
#include "Input.hpp"
#include "../libs/get/src/Utils.hpp"
#include "../libs/get/src/Get.hpp"

int console_main(Get* get)
{
	// initialize text console
	Console* console = new Console();

	Input* input = new Input();

	// create main menu object
	Menu* menu = new Menu(console, get);

	bool running = true;

	while(running)
	{
		console->background(42, 37, 39);

		// show the current menu screen
		menu->display();
		SDL_Delay(16);

		// update pressed buttons in input object
		input->updateButtons();

		// if we're on the install screen, perform an install
		if (menu->screen == INSTALLING || menu->screen == REMOVING)
		{
			Package* target = get->packages[menu->position];

			// install package
			bool succeeded = false;

			if (menu->screen == INSTALLING)
				succeeded = get->install(target);
			else if (menu->screen == REMOVING)
				succeeded = get->remove(target);

			// change screen accordingly
			if (succeeded)
				menu->screen = INSTALL_SUCCESS;
			else
				menu->screen = INSTALL_FAILED;
		}

		// send either A or B to the menu object, if held
		if (input->held(BUTTON_A) || input->held(BUTTON_B))
			menu->advanceScreen(input->held(BUTTON_A));

		if (menu->screen == INSTALL_SCREEN && input->held(BUTTON_X))
			menu->screen = REMOVING;

		// if minus is pressed, exit
		if (input->held(BUTTON_MINUS))
			running = false;

		// if B is pressed on the splash screen, exit
		//		if (menu->screen == SPLASH && input->held(BUTTON_B))
		//			running = false;

		// move cursor up or down depending on input
		menu->moveCursor(-1*(input->held(BUTTON_UP)) + (input->held(BUTTON_DOWN)));

		// move page PAGE_SIZE forward/backward depending on input
		menu->moveCursor(-1*PAGE_SIZE*input->held(BUTTON_LEFT) + PAGE_SIZE*input->held(BUTTON_RIGHT));
	}

	console->close();
	input->close();

	return 0;
}
