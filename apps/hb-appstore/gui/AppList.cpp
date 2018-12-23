#include "MainDisplay.hpp"
#include "AppCard.hpp"
#include "Button.hpp"
#include "AboutScreen.hpp"
#include "Keyboard.hpp"
#include <SDL2/SDL2_gfxPrimitives.h>
#include <algorithm>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand


AppList::AppList(Get* get, Sidebar* sidebar)
{
    this->x = 400 - 260*(R-3);

	// the offset of how far along scroll'd we are
	this->y = 0;

	// the main get instance that contains repo info and stuff
	this->get = get;

	// the sidebar, which will store the currently selected category info
	this->sidebar = sidebar;

  // initialize random numbers used for sorting
  std::srand ( unsigned ( std::time(0) ) );

	// update current app listing
	update();
}

int myrandom (int i) { return std::rand()%i;}

bool AppList::process(InputEvents* event)
{
    bool ret = false;

    if (event->pressed(Z_BUTTON) || event->pressed(L_BUTTON))
    {
        R = (R==3)? 4 : 3;
        this->x = 400 - 260*(R-3);
        update();
        return true;
    }
    
    // must be done before keyboard stuff to properly switch modes
    if (event->isTouchDown())
    {
        // remove a highilight if it exists (TODO: same as an above if statement)
        if (this->highlighted >= 0 && this->highlighted < this->elements.size() && this->elements[this->highlighted])
            this->elements[this->highlighted]->elasticCounter = NO_HIGHLIGHT;
        
        // got a touch, so let's enter touchmode
        this->highlighted = -1;
        this->touchMode = true;
    }

    // if we're showing a keyboard, make sure we're not in its bounds
    // also make sure the children elements exist before trying the keyboard
    // AND we're actually on the search category
    // also if we're not in touchmode, always go in here regardless of any button presses (user can only interact with keyboard)
    bool keyboardIsShowing = this->elements.size() > 0 &&
        this->sidebar != NULL && this->sidebar->curCategory == 0 &&
        this->keyboard != NULL && !this->keyboard->hidden;
    if (keyboardIsShowing &&
        ((event->isTouchDown() && event->touchIn(keyboard->x, keyboard->y,
                       keyboard->width, keyboard->height)) || !touchMode))
    {
        ret |= this->keyboard->process(event);
        if (event->isKeyDown() && event->held(Y_BUTTON))
            ret |= ListElement::process(event); // continue processing ONLY if they're pressing Y
        return ret;
    }

    int origHighlight = this->highlighted;

    // process some joycon input events
    if (event->isKeyDown())
    {
        if (keyboardIsShowing)
        {
            // keyboard is showing, but we'r epressing buttons, and we're down here, so set touch mode and get out
            touchMode = false;
            if (event->held(Y_BUTTON))  // again, only let a Y through to toggle keyboard (TODO: redo this!)
                ret |= ListElement::process(event);
            return true;    // short circuit, should be handled by someone else
        }
        
        if (event->held(A_BUTTON | B_BUTTON | UP_BUTTON | DOWN_BUTTON | LEFT_BUTTON | RIGHT_BUTTON))
        {
            // if we were in touch mode, draw the cursor in the applist
            // and reset our position
            if (this->touchMode)
            {
                this->touchMode = false;
                this->highlighted = 0;
                this->y = 0;		// reset scroll TODO: maintain scroll when switching back from touch mode
                event->keyCode = -1;  // we already have the cursor where we want it, no further updates
                ret |= true;
            }

            if (event->held(A_BUTTON) && this->highlighted >= 0)
            {
              this->elements[this->highlighted]->action();
              ret |= true;
            }

            // touchmode is false, but our highlight value is negative
            // (do nothing, let sidebar update our highlight value)
            if (this->highlighted < 0) return false;

            // look up whatever is currently chosen as the highlighted position
            // and remove its highlight
            if (this->elements[this->highlighted])
                this->elements[this->highlighted]->elasticCounter = NO_HIGHLIGHT;

            // if we got a LEFT key while on the left most edge already, transfer to categories
            if (this->highlighted%R==0 && event->held(LEFT_BUTTON))
            {
                this->highlighted = -1;
                this->sidebar->highlighted = this->sidebar->curCategory;
                return true;
            }

            // similarly, prevent a RIGHT from wrapping to the next line
            if (this->highlighted%R==(R-1) && event->held(RIGHT_BUTTON)) return false;

            // adjust the cursor by 1 for left or right
            this->highlighted += -1*(event->held(LEFT_BUTTON)) + (event->held(RIGHT_BUTTON));

            // adjust it by R for up and down
            this->highlighted += -1*R*(event->held(UP_BUTTON)) + R*(event->held(DOWN_BUTTON));

            // don't let the cursor go out of bounds
            if (this->highlighted >= (int)this->elements.size()) this->highlighted = this->elements.size() - 1;

            if (this->highlighted < 0) this->highlighted = 0;
            if (this->highlighted >= (int)this->totalCount) this->highlighted = this->totalCount-1;
        }
    }
    
    // always check the currently highlighted piece and try to give it a thick border or adjust the screen
    if (!touchMode && this->elements.size() > this->highlighted && this->highlighted >= 0 && this->elements[this->highlighted])
    {
        // if our highlighted position is large enough, force scroll the screen so that our cursor stays on screen
        
        Element* curTile = this->elements[this->highlighted];
        
        // the y-position of the currently highlighted tile, precisely on them screen (accounting for scroll)
        // this means that if it's < 0 or > 720 then it's not visible
        int normalizedY = curTile->y + this->y;
        
        // if we're out of range above, recenter at the top row
        if (normalizedY < 0)
            this->y = -1 * (curTile->y - 15) + 25;
        
        // if we're out of range below, recenter at bottom row
        if (normalizedY > 720 - curTile->height)
            this->y = -1 * (curTile->y - 3*(curTile->height - 15)) - 40;
        
        // if the card is this close to the top, just set it the list offset to 0 to scroll up to the top
        if (this->highlighted < R)
            this->y = 0;
        
        if (this->elements[this->highlighted])
            this->elements[this->highlighted]->elasticCounter = THICK_HIGHLIGHT;
    }

    // highlight was modified, we need to redraw
    if (origHighlight != this->highlighted)
        ret |= true;

	ret |= ListElement::process(event);

	return ret;
}

void AppList::render(Element* parent)
{
	if (this->parent == NULL)
		this->parent = parent;

	// draw a white background, 870 wide
	SDL_Rect dimens = { 0, 0, 920 + 260*(R-3), 720 };
	dimens.x = this->x - 35;

	SDL_SetRenderDrawColor(parent->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(parent->renderer, &dimens);
	this->renderer = parent->renderer;

	super::render(this);
}

void AppList::update()
{
    // if there's a keyboard, get its current highlighted positions
    int kRow = -1, kIndex = -1;
    if (this->keyboard) {
        kRow = keyboard->curRow;
        kIndex = keyboard->index;
    }
    
	// remove any old elements
	this->wipeElements();

	// quickly create a vector of "sorted" apps
	// (they must be sorted by UPDATE -> INSTALLED -> LOCAL -> GET)
	// TODO: sort this a better way, and also don't use 3 distinct for loops
	std::vector<Package*> sorted;

	// the current category value from the sidebar
	std::string curCategoryValue = this->sidebar->currentCatValue();

	// all packages TODO: move some of this filtering logic into main get library
	std::vector<Package*> packages = get->packages;

	// if it's a search, do a search query through get rather than using all packages
	if (curCategoryValue == "_search")
		packages = get->search(this->sidebar->searchQuery);

    // sort the packages list by whatever criteria is currently set
    const char* sortString = applySortOrder(&packages);

    if (this->sortMode == RECENT)
    {
        // alphabetical sort order is the default view, so it puts updates and installed apps first

        // update
        for (int x=0; x<packages.size(); x++)
            if (packages[x]->status == UPDATE)
                sorted.push_back(packages[x]);

        // installed
        for (int x=0; x<packages.size(); x++)
            if (packages[x]->status == INSTALLED)
                sorted.push_back(packages[x]);

        // local
        for (int x=0; x<packages.size(); x++)
            if (packages[x]->status == LOCAL)
                sorted.push_back(packages[x]);

        // get
        for (int x=0; x<packages.size(); x++)
            if (packages[x]->status == GET)
                sorted.push_back(packages[x]);
    }
    else
    {
        // not alphabetical, just copy over to the sorted vector
        for (int x=0; x<packages.size(); x++)
            sorted.push_back(packages[x]);
    }

	// total apps we're interested in so far
	int count = 0;

	for (int x=0; x<sorted.size(); x++)
	{
		// if we're on all categories, or this package matches the current category (or it's a search (prefiltered))
		if (curCategoryValue == "_all" || curCategoryValue == sorted[x]->category || curCategoryValue == "_search")
		{
			AppCard* card = new AppCard(sorted[x]);
			card->index = count;

			this->elements.push_back(card);

			// we drew an app, so increase the displayed app counter
			count ++;
		}
	}

	this->totalCount = count;

	// position the filtered app card list
	for (int x=0; x<this->elements.size(); x++)
	{
		// every element after the first should be an app card (we just added them)
		AppCard* card = (AppCard*) elements[x];

		// position at proper x, y coordinates
		card->position(25 + (x%R)*265, 145 + (card->height+15)*(x/R));		// TODO: extract formula into method (see above)
		card->update();
	}

	// the title of this category (from the sidebar)
	SDL_Color black = { 0, 0, 0, 0xff };
	TextElement* category;

	// if it's a search, add a keyboard
	if (curCategoryValue == "_search")
	{
		this->keyboard = new Keyboard(this, &this->sidebar->searchQuery);
        if (kRow >= 0 || kIndex >=0)
        {
            this->keyboard->curRow = kRow;
            this->keyboard->index = kIndex;
        }
		this->elements.push_back(keyboard);

		category = new TextElement((std::string("Search: \"") + this->sidebar->searchQuery + "\"").c_str(), 28, &black);
	}
	else
	{
		category = new TextElement(this->sidebar->currentCatName().c_str(), 28, &black);
	}

	category->position(20, 90);
	this->elements.push_back(category);

    // additional buttons (only if not on search)
    if (curCategoryValue != "_search")
    {
        Button* settings = new Button("Credits", X_BUTTON, false, 15);
        settings->position(700 + 260*(R-3), 70);
        settings->action = std::bind(&AppList::launchSettings, this);
        this->elements.push_back(settings);

        Button* sort = new Button("Adjust Sort", Y_BUTTON, false, 15);
        sort->position(settings->x - 20 - sort->width, settings->y);
        sort->action = std::bind(&AppList::cycleSort, this);
        this->elements.push_back(sort);


          // display the search type above if it's not the default one
          SDL_Color gray = {0x50, 0x50, 0x50, 0xff};
         TextElement* sortBlurb = new TextElement(sortString, 15, &gray);
         sortBlurb->position(category->x + category->width + 15, category->y + 12);
         this->elements.push_back(sortBlurb);
    }
    else
    {
        Button* settings = new Button("Toggle Keyboard", Y_BUTTON, false, 15);
        settings->position(625 + 260*(R-3), 70);
        settings->action = std::bind(&AppList::toggleKeyboard, this);
        this->elements.push_back(settings);
    }
}

const char* AppList::applySortOrder(std::vector<Package*>* p)
{
    if (sortMode == ALPHABETICAL)
        std::sort(p->begin(), p->end(),
                  [] (const auto& lhs, const auto& rhs) {
                      return lhs->title.compare(rhs->title) < 0;
                  });
    else if (sortMode == POPULARITY)
        std::sort(p->begin(), p->end(),
                  [] (const auto& lhs, const auto& rhs) {
                      return lhs->downloads > rhs->downloads;
                  });
    else if (sortMode == RECENT)
        std::sort(p->begin(), p->end(),
                  [] (const auto& lhs, const auto& rhs) {
                      return lhs->updated_timestamp > rhs->updated_timestamp;
                  });
    else if (sortMode == SIZE)
        std::sort(p->begin(), p->end(),
                  [] (const auto& lhs, const auto& rhs) {
                      return lhs->download_size > rhs->download_size;
                  });
    else if (sortMode == RANDOM)
    {
        std::random_shuffle(p->begin(), p->end(), myrandom);
    }

    const char* humanStrings[] = {"by most recent", "by download count", "alphabetically", "by size (descending)", "randomly"};
    return humanStrings[sortMode];
}

void AppList::reorient()
{
    // remove a highilight if it exists (TODO: extract method, we use this everywehre)
    if (this->highlighted >= 0 && this->highlighted < this->elements.size() && this->elements[this->highlighted])
        this->elements[this->highlighted]->elasticCounter = NO_HIGHLIGHT;
}

void AppList::cycleSort()
{
    reorient();
    this->sortMode = (this->sortMode + 1) % TOTAL_SORTS;
    this->update();
}

void AppList::toggleKeyboard()
{
    if (this->keyboard)
    {
        reorient();
        this->keyboard->hidden = !this->keyboard->hidden;
        
        // if it's hidden now, make sure we release our highlight
        if (this->keyboard->hidden)
        {
            this->sidebar->highlighted = -1;
            this->highlighted = 0;
        }
        
        this->needsRedraw = true;
    }
    
}

void AppList::launchSettings()
{
    MainDisplay::subscreen = new AboutScreen(this->get);
}
