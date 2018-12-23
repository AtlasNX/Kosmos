#include "Menu.hpp"
#include <sstream>
#include <algorithm>

using namespace std;

Menu::Menu(Console* console, Get* get)
{
	this->console = console;
	this->get = get;
	
	this->position = 0;
	
	this->screen = SPLASH;
	
	if (get->packages.size() > 0)
		this->repoUrl = get->packages[0]->repoUrl->c_str();
	else
		this->repoUrl = "No packages found on any repos!";
}

void Menu::display()
{
	if (this->screen == SPLASH)
	{
		console->drawString(9, 21, "Homebrew App Store (Preview!)");
		console->drawColorString(9, 26, "thanks to:", 0xcc, 0xcc, 0xcc);
		console->drawColorString(15, 27, "vgmoose, pwsincd, rw-r-r_0644, zarklord", 0xcc, 0xcc, 0xcc);
		console->drawColorString(9, 32, "Press [A] to continue", 0xff, 0xff, 0x00);
	}
	
	if (this->screen == LIST_MENU || this->screen == INSTALL_SCREEN)
	{
		// draw the top bar
		console->fillRect(0, 0, 80, 1, 255, 255, 0);
		console->drawColorString(80/2 - 15, 0, "Homebrew App Store Preview", 0, 0, 0);
		
		// draw bottom bar
		console->fillRect(0, 44, 80, 1, 255, 255, 0);
		console->drawColorString(0, 44, "[A] Install   [B] Back", 0, 0, 0);
		console->drawColorString(80-(strlen(this->repoUrl)+2), 44, this->repoUrl, 0, 0, 0);
	}
	
	if (this->screen == LIST_MENU)
	{
		int start = (this->position/PAGE_SIZE)*PAGE_SIZE; //
		
		// go through this page of apps until the end of the page, or longer than the packages list
		for (int x=start; x<start+PAGE_SIZE && x < get->packages.size(); x++)
		{
			int curPosition = (x % PAGE_SIZE)*3 + 2;
			
			Package* cur = get->packages[x];
			std::stringstream line;
			line << cur->title << " (" << cur->version << ")";
			console->drawString(15, curPosition, line.str().c_str());

			int r = (cur->status == UPDATE || cur->status == LOCAL)? 0xFF : 0x00;
			int g = (cur->status == UPDATE)? 0xF7 : 0xFF;
			int b = (cur->status == INSTALLED || cur->status == LOCAL)? 0xFF : 0x00;
			console->drawColorString(5, curPosition, cur->statusString(), r, g, b);
			
			std::stringstream line2;
			line2  << cur->short_desc << " [" << cur->author << "]";
			
			console->drawColorString(16, curPosition + 1, line2.str().c_str(), 0xcc, 0xcc, 0xcc);
		}
		
		std::stringstream footer;
		footer << "Page " << this->position/PAGE_SIZE+1 << " of " << get->packages.size()/PAGE_SIZE+1;
		console->drawString(34, 40, footer.str().c_str());
		console->drawColorString(15, 42, "Use left/right and up/down to switch pages and apps", 0xcc, 0xcc, 0xcc);

		console->drawString(1, (this->position % PAGE_SIZE)*3 + 2, "-->");
	}
	
	if (this->screen == INSTALL_SCREEN)
	{
		if (this->position < 0 || this->position > get->packages.size())
		{
			// invalid selection, go back a screen
			this->screen--;
			return;
		}
		
		// currently selected package
		Package* cur = get->packages[this->position];
		
		console->drawString(5, 3, cur->title.c_str());
		console->drawString(6, 5, cur->version.c_str());
		console->drawString(6, 6, cur->author.c_str());
		
		int r = (cur->status == UPDATE || cur->status == LOCAL)? 0xFF : 0x00;
		int g = (cur->status == UPDATE)? 0xF7 : 0xFF;
		int b = (cur->status == INSTALLED || cur->status == LOCAL)? 0xFF : 0x00;
		console->drawColorString(5, 8, cur->statusString(), r, g, b);
		
		console->drawColorString(5, 12, "Press [A] to install this package", 0xff, 0xff, 0x00);
		
		if (cur->status != GET)
			console->drawColorString(5, 14, "Press [X] to remove this package", 0xff, 0xff, 0x00);
		
		console->drawString(5, 16, "Press [B] to go back");
	}
	
	if (this->screen == INSTALLING || this->screen == REMOVING)
	{
		// currently selected package
		Package* cur = get->packages[this->position];
		
		console->drawString(5, 4, cur->title.c_str());
		
		if (this->screen == INSTALLING)
			console->drawColorString(5, 5, "Downloading package...", 0xff, 0xff, 0x00);
		else
			console->drawColorString(5, 5, "Removing package...", 0xff, 0xff, 0x00);
		
		console->drawString(5, 6, "No progress bar yet!");
		console->drawString(5, 7, "github.com/vgmoose/appstorenx/issues");
	}
	
	if (this->screen == INSTALL_SUCCESS || this->screen == INSTALL_FAILED)
	{
		if (this->screen == INSTALL_SUCCESS)
			console->drawColorString(3, 12, "Operation successful!", 0, 0xff, 0);
		else
		{
			console->drawColorString(3, 11, "Operation failed", 0xff, 0, 0);
			console->drawString(3, 12, "You can file an issue at github.com/vgmoose/appstorenx");
		}
		
		console->drawColorString(3, 14, "Press [A] to continue", 0xff, 0xff, 0x00);
	}
	
	console->update();
}

void Menu::moveCursor(int diff)
{
	int old_position = position;
	this->position += diff;
	
	if (position < 0)
	{
		// went back too far, wrap around to last package
		position = get->packages.size()-1;
	}
	
	else if (position >= get->packages.size())
	{
		// too far forward, wrap around to first package
		position = 0;
	}
}

void Menu::advanceScreen(bool advance)
{
	if (advance)
	{
		// A on these screens just returns to app list
		if (this->screen == INSTALL_SUCCESS || this->screen == INSTALL_FAILED)
		{
			this->screen = LIST_MENU;
		}
		// if on the install screen, or install-related screens, A does nothing
		else if (this->screen < INSTALLING)
		{
			// just advance the screen
			this->screen ++;
		}
	}
	else
	{
		// go back, unless we can't anymore, or doing an install-related action
		if (this->screen > SPLASH && this->screen < INSTALLING)
			this->screen--;
	}
	printf("current screen: %d\n", this->screen);
}
