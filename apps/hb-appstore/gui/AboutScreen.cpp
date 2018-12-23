#include "AboutScreen.hpp"
#include "../libs/get/src/Get.hpp"
#include "../libs/get/src/Utils.hpp"
#include "Button.hpp"
#include <SDL2/SDL2_gfxPrimitives.h>
#include <sstream>
#include "MainDisplay.hpp"
#include "Feedback.hpp"

AboutScreen::AboutScreen(Get* get)
{
    this->get = get;

    SDL_Color red = {0xFF, 0x00, 0x00, 0xff};
    SDL_Color gray = {0x50, 0x50, 0x50, 0xff};
    SDL_Color black = {0x00, 0x00, 0x00, 0xff};
    SDL_Color white = {0xFF, 0xFF, 0xFF, 0xff};

    // TODO: show current app status somewhere

    // download/update/remove button (2)

    Button* cancel = new Button("Go Back", B_BUTTON, true, 30);
    cancel->position(30, 30);
    cancel->action = std::bind(&AboutScreen::back, this);
    this->elements.push_back(cancel);

    Button* cleanup = new Button("Cleanup Empty Folders", Y_BUTTON, true, 21);
    cleanup->position(30, 500);
    cleanup->action = std::bind(&AboutScreen::removeEmptyFolders, this);
    this->elements.push_back(cleanup);

    Button* cache = new Button("Delete Image Cache", X_BUTTON, true, 21, cleanup->width);
    cache->position(30, cleanup->y + cleanup->height + 25);
    cache->action = std::bind(&AboutScreen::wipeCache, this);
    this->elements.push_back(cache);

    int MARGIN = 550;

    Button* feedback = new Button("Leave Feedback", A_BUTTON, false, 17);
    feedback->position(MARGIN + 450, 65);
    feedback->action = std::bind(&AboutScreen::launchFeedback, this);
    this->elements.push_back(feedback);

    TextElement* title = new TextElement("Homebrew App Store", 35, &black);
    title->position(MARGIN, 40);
    this->elements.push_back(title);

    TextElement* subtitle = new TextElement("by fortheusers.org", 25, &black);
    subtitle->position(MARGIN, 80);
    this->elements.push_back(subtitle);

    const char* blurb = "Licensed under the GPLv3 license.\n\nThis app is free and open source because the users (like you!) deserve it. Let's support homebrew and the right to control what software we run on our own devices!\n\n\nCredits:\nVGMoose, Pwsincd, rw-r-r_0644, Zarklord1, Maschell, Roman, quarktheawesome, Whovian9369, Ep8Script, crc-32, rakujira\n\n\n";

#if !defined(__WIIU__)
    const char* platform_blurb = "Also FYI you can browse the Internet directly on your Switch by entering 45.55.142.122 as a manual DNS in connection settings. A full web browser is built into the console, but it's normally hidden and inaccessible to the end user.";
#else
    const char* platform_blurb = "Thank you for 2 years of Wii U homebrew! This update is possible due to a Wii U port of the SDL2 library. Making Wii U homebrew has never been easier, if you're interested, find us on Discord at https://discordapp.com/invite/F2PKpEj";
#endif

    TextElement* credits = new TextElement((std::string(blurb) + platform_blurb).c_str(), 20, &black, false, 600);
    credits->position(MARGIN, 150);
    this->elements.push_back(credits);

}

void AboutScreen::render(Element* parent)
{
    if (this->parent == NULL)
        this->parent = parent;

    // draw a white background, 870 wide
    SDL_Rect dimens = { 400, 0, 1280-400, 720 };

    SDL_SetRenderDrawColor(parent->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(parent->renderer, &dimens);
    this->renderer = parent->renderer;

    super::render(this);
}

void AboutScreen::back()
{
    MainDisplay::subscreen = NULL;      // TODO: clean up memory?
}

void AboutScreen::removeEmptyFolders()
{
    remove_empty_dirs(ROOT_PATH, 0);
}

void AboutScreen::wipeCache()
{
    // clear out versions
    std::remove(".get/tmp/cache/versions.json");
}

void AboutScreen::launchFeedback()
{
    // find the package corresponding to us
    for (auto& package : this->get->packages)
    {
        if (package->pkg_name == "appstore")
        {
            MainDisplay::subscreen = new Feedback(package);
            break;
        }
    }
}
