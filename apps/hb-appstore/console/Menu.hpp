#include "../libs/get/src/Get.hpp"
#include "Console.hpp"

#define SPLASH 1
#define CATEGORIES N/A
#define LIST_MENU 2
#define INSTALL_SCREEN 3

#define INSTALLING 4
#define INSTALL_SUCCESS 5
#define INSTALL_FAILED 6

#define REMOVING 7

// number of apps on one page
#define PAGE_SIZE 12

class Menu
{
public:
	Menu(Console* console, Get* get);
	void display();			// display the menu as it's set up
	void moveCursor(int diff);	// move the cursor position up (-1) or down (1)
	void advanceScreen(bool advance);		// change the active screen to the next one
	int screen;		// current screen
	int position;	// the position of the cursor along the current menu
	
private:
	int offset;			// the offset of "scroll" along the current menu page
	Get* get;		// list of packages to draw
	
	const char* repoUrl;
	
	Console* console;	// link to the console
};
