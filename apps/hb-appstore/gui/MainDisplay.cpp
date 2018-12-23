#include "MainDisplay.hpp"
#include "AppCard.hpp"
#include "../libs/get/src/Utils.hpp"

#if defined(SWITCH)
#include <switch.h>
#endif

#if defined(__WIIU__)
#include <romfs-wiiu.h>
#endif

SDL_Renderer* MainDisplay::mainRenderer = NULL;
Element* MainDisplay::subscreen = NULL;
MainDisplay* MainDisplay::mainDisplay = NULL;

MainDisplay::MainDisplay(Get* get)
{
	this->get = get;

	// populate image cache with any local version info if it exists
	this->imageCache = new ImageCache(get->tmp_path);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
//        printf("SDL init failed: %s\n", SDL_GetError());
		return;
	}

	if (TTF_Init() < 0) {
//        printf("SDL ttf init failed: %s\n", SDL_GetError());
		return;
	}

	int imgFlags = IMG_INIT_PNG;
	if( !( IMG_Init( imgFlags ) & imgFlags ) )
	{
//        printf("SDL image init failed: %s\n", SDL_GetError());
		return;
	}
    
    // initialize teh romfs for switch/wiiu
#if defined(SWITCH) || defined(__WIIU__)
    romfsInit();
#endif

//    printf("initialized SDL\n");

	int height = 720;
	int width = 1280;

	this->window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
	this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_SOFTWARE);

	//Detach the texture
	SDL_SetRenderTarget(this->renderer, NULL);

	MainDisplay::mainRenderer = this->renderer;
    MainDisplay::mainDisplay = this;

	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_JoystickOpen(i) == NULL) {
//                printf("SDL_JoystickOpen: %s\n", SDL_GetError());
				SDL_Quit();
				return;
		}
	}
    
    // set up the SDL needsRender event
    this->needsRender.type = SDL_USEREVENT;
    
    // go through all repos and if one has an error, set the error flag
    bool atLeastOneEnabled = false;
    for (auto repo : this->get->repos)
    {
        this->error = this->error || !repo->loaded;
        atLeastOneEnabled = atLeastOneEnabled || repo->enabled;
    }
    
    this->error = this->error || !atLeastOneEnabled;

	// the progress bar
	ProgressBar* pbar = new ProgressBar();
	pbar->position(401, 380 - this->error*290);
	this->elements.push_back(pbar);

	// the text above the progress bar
//	TextElement* pbar_text = new TextElement("Updating App Info...", 17);
//	pbar_text->position(550, 365);
//	this->elements.push_back(pbar_text);

	// create the first two elements (icon and app title)
	ImageElement* icon = new ImageElement(ROMFS "res/icon.png");
	icon->position(330 + this->error*140, 255 - this->error*230);
	icon->resize(70 - this->error*35, 70 - this->error*35);
	this->elements.push_back(icon);

	TextElement* title = new TextElement("Homebrew App Store", 50 - this->error*25);
	title->position(415 + this->error*100, 255 - this->error*230);
	this->elements.push_back(title);
    
    if (this->imageCache->version_cache.size() == 0)
    {
        notice = new TextElement("Still doing initial load-- next time will be faster!", 20);
        notice->position(410, 460);
        notice->hidden = true;
        this->elements.push_back(notice);
    }
    
    if (this->error)
    {
        std::string troubleshootingText = "No enabled repos found, check ./get/repos.json\nMake sure repo has at least one package";
        if (atLeastOneEnabled)
            troubleshootingText = std::string("Perform a connection test in the Switch System Settings\nEnsure DNS isn't blocking: ") + this->get->repos[0]->url;
        
        TextElement* errorMessage = new TextElement("Couldn't connect to the Internet!", 40);
        errorMessage->position(345, 305);
        this->elements.push_back(errorMessage);
        
        TextElement* troubleshooting = new TextElement((std::string("Troubleshooting:\n") + troubleshootingText).c_str(), 20, NULL, false, 600);
        troubleshooting->position(380, 585);
        this->elements.push_back(troubleshooting);
    }
}

bool MainDisplay::process(InputEvents* event)
{
	// if we're on the splash/loading screen, we need to fetch icons+screenshots from the remote repo
	// and load them into our surface cache with the pkg_name+version as the key
	if (this->showingSplash && event->noop)
	{
		// should be a progress bar
		if (this->get->packages.size() != 1)
			((ProgressBar*)this->elements[0])->percent = (this->count / ((float)this->get->packages.size()-1));
        
		// no packages, prevent crash TODO: display offline in bottom bar
		if (this->get->packages.size() == 0)
		{
			((ProgressBar*)this->elements[0])->percent = -1;
            this->showingSplash = false;
            return true;
		}
        
        if (notice && ((ProgressBar*)this->elements[0])->percent > 0.5)
            notice->hidden = false;
        
        // update the counter (TODO: replace with fetching app icons/screen previews)
        this->count++;

		// get the package whose icon+screen to process
		Package* current = this->get->packages[this->count - 1];

		// the path to the cache location of the icon and screen for this pkg_name and version number
		std::string key_path = imageCache->cache_path + current->pkg_name;

		// check if this package exists in our cache, but the version doesn't match
		// (if (it's not in the cache) OR (it's in the cache but the version doesn't match)
		if (this->imageCache->version_cache.count(current->pkg_name) == 0 ||
			(this->imageCache->version_cache.count(current->pkg_name) &&
			 this->imageCache->version_cache[current->pkg_name] != current->version))
		{
			// the version in our cache doesn't match the one that will be on the server
			// so we need to download the images now
			mkdir(key_path.c_str(), 0700);

			bool success = downloadFileToDisk(*(current->repoUrl) + "/packages/" + current->pkg_name + "/icon.png", key_path + "/icon.png");
			if (!success) // manually add defualt icon to cache if downloading failed
				cp(ROMFS "res/default.png", (key_path + "/icon.png").c_str());
            // TODO: generate a custom icon for this version with a color and name
            
            success = downloadFileToDisk(*(current->repoUrl) + "/packages/" + current->pkg_name + "/screen.png", key_path + "/screen.png");
            if (!success)
                cp(ROMFS "res/noscreen.png", (key_path + "/screen.png").c_str());

			// add these versions to the version map
			this->imageCache->version_cache[current->pkg_name] = current->version;
		}

		// whether we just downloaded it or it was already there from the cache, load this image element into our memory cache
		// (making an AppCard and calling update() will do this, even if we don't intend to do anything with it yet)
		AppCard a(current);
		a.update();

		// write the version we just got to the cache as well so that we can know whether or not we need to up date it next time

		// are we done processing all packages
		if (this->count == this->get->packages.size())
		{
			// write whatever we have in the icon version cache to a file
			this->imageCache->writeVersionCache();

			// remove the splash screen elements
			this->wipeElements();

			// add in the sidebar, footer, and main app listing
			Sidebar* sidebar = new Sidebar();
			this->elements.push_back(sidebar);
            
			AppList* applist = new AppList(this->get, sidebar);
			this->elements.push_back(applist);
			sidebar->appList = applist;

			this->showingSplash = false;
            this->needsRedraw = true;
		}
        
        return true;
	}
	else
	{
        if (MainDisplay::subscreen)
            return MainDisplay::subscreen->process(event);
		// keep processing child elements
		return super::process(event);
	}

	return false;
}

void MainDisplay::render(Element* parent)
{
    // set the background color
    MainDisplay::background(0x42, 0x45, 0x48);
//    MainDisplay::background(0x60, 0x7d, 0x8b);
#if defined(__WIIU__)
    MainDisplay::background(0x54, 0x55, 0x6e);
#endif
    
    if (MainDisplay::subscreen)
    {
        MainDisplay::subscreen->render(this);
        this->update();
        return;
    }

	// render the rest of the subelements
	super::render(this);

	// commit everything to the screen
	this->update();
}

void MainDisplay::background(int r, int g, int b)
{
	SDL_SetRenderDrawColor(this->renderer, r, g, b, 0xFF);
	SDL_RenderFillRect(this->renderer, NULL);
}

void MainDisplay::update()
{
    // never exceed 60fps because there's no point
    
//    int now = SDL_GetTicks();
//    int diff = now - this->lastFrameTime;
//
//    if (diff < 16)
//        return;

    SDL_RenderPresent(this->renderer);
//    this->lastFrameTime = now;
}

void quit()
{
    IMG_Quit();
    TTF_Quit();
    
    SDL_Delay(10);
    SDL_DestroyWindow(MainDisplay::mainDisplay->window);
    
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
    
#if defined(__WIIU__)
    romfsExit();
#endif

#if defined(SWITCH)
    socketExit();
#endif
    exit(0);
}
