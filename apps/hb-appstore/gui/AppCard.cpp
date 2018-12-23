#include "AppCard.hpp"
#include "MainDisplay.hpp"

AppCard::AppCard(Package* package)
{
	this->package = package;

	// fixed width+height of one app card
	this->width = 256;
	this->height = 195;
    
#if defined(__WIIU__)
    this->height = 135;
#endif
    
    this->touchable = true;
    
    // connect the action to the callback for this element, to be invoked when the touch event fires
    this->action = std::bind(&AppCard::displaySubscreen, this);
}

void AppCard::update()
{
	// create the layout of the app card (all relative)

	// icon, and look up cached image to load
	ImageElement* icon = new ImageElement((ImageCache::cache_path + this->package->pkg_name + "/icon.png").c_str());
	icon->position(this->x, this->y);
	icon->resize(256, this->height - 45);
    
	this->elements.push_back(icon);

	int size = 13;

	SDL_Color gray = { 80, 80, 80, 0xff };
	SDL_Color black = { 0, 0, 0, 0xff };

	// version
	TextElement* version = new TextElement(("v. " + package->version).c_str(), size, &gray);
	version->position(this->x + 40, this->y + icon->height + 10);
	this->elements.push_back(version);

	// status string
	TextElement* status = new TextElement(package->statusString(), size, &gray);
	status->position(this->x + 40, this->y + icon->height + 25);
	this->elements.push_back(status);

	// app name
	int w, h;
	TextElement* appname = new TextElement(package->title.c_str(), size+3, &black);
	SDL_QueryTexture(appname->textSurface, NULL, NULL, &w, &h);
	appname->position(this->x + 245 - w, this->y + icon->height + 5);
	this->elements.push_back(appname);

	// author
	TextElement* author = new TextElement(package->author.c_str(), size, &gray);
	SDL_QueryTexture(author->textSurface, NULL, NULL, &w, &h);
	author->position(this->x + 245 - w, this->y + icon->height + 25);
	this->elements.push_back(author);

	// download status icon
	ImageElement* statusicon = new ImageElement((ROMFS "res/" + std::string(package->statusString()) + ".png").c_str());
	statusicon->position(this->x + 4, this->y + icon->height + 10);
	statusicon->resize(30, 30);
	this->elements.push_back(statusicon);
}

void AppCard::render(Element* parent)
{
	// grab and store the parent while we have it, and if we need it
	if (this->parent == NULL)
    {
		this->parent = parent;
        this->xOff = this->parent->x;
        this->yOff = this->parent->y;
    }

	// TODO: don't render this card if it's going to be offscreen anyway according to the parent (AppList)
//	if (((AppList*)parent)->scrollOffset)

	// render all the subelements of this card
	super::render(parent);
}

void AppCard::displaySubscreen()
{
    // received a click on this app, add a subscreen under the parent
    // (parent of AppCard should be AppList)
    if (!this->parent) return;
    
    AppList* appList = ((AppList*)this->parent);
    MainDisplay::subscreen = new AppDetails(this->package, appList);
    if (!appList->touchMode)
        ((AppDetails*)MainDisplay::subscreen)->highlighted = 0;        // show cursor if we're not in touch mode
}

bool AppCard::process(InputEvents* event)
{
	if (this->parent == NULL)
		return false;
    
    this->xOff = this->parent->x;
    this->yOff = this->parent->y;
    
    return super::process(event);
}
