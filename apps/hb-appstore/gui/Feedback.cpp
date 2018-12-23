#include "Feedback.hpp"
#include "TextElement.hpp"
#include "ImageElement.hpp"
#include "Button.hpp"
#include "MainDisplay.hpp"
#include "AppCard.hpp"

#include <curl/curl.h>
#include <curl/easy.h>

Feedback::Feedback(Package* package)
{
    this->package = package;
    
    this->message = std::string("");

    this->refresh();
}

void Feedback::refresh()
{
    // if there's a keyboard, get its current highlighted positions
    int kRow = -1, kIndex = -1;
    if (this->keyboard) {
        kRow = keyboard->curRow;
        kIndex = keyboard->index;
    }
    
    this->wipeElements();
    
    TextElement* elem = new TextElement((std::string("Leaving feedback for: \"") + package->title + "\"").c_str(), 25);
    elem->position(50, 30);
    elements.push_back(elem);
    
    AppCard card(package);
    
    ImageElement* icon = new ImageElement((ImageCache::cache_path + package->pkg_name + "/icon.png").c_str());
    icon->position(50, 160);
    icon->resize(256, card.height - 45); // TODO: extract method for icon height, in common with wiiu/switch
    elements.push_back(icon);
    
    TextElement* feedback = new TextElement(this->message.c_str(), 23, NULL, false, 730);
    feedback->position(390, 140);
    elements.push_back(feedback);
    
    this->keyboard = new Keyboard(NULL, &this->message, this);
    if (kRow >= 0 || kIndex >=0)
    {
        this->keyboard->curRow = kRow;
        this->keyboard->index = kIndex;
        this->keyboard->touchMode = false;
    }
    this->keyboard->x = 200;
    elements.push_back(keyboard);
    
    Button* send = new Button("Submit", X_BUTTON, true, 24);
    Button* quit = new Button("Discard", Y_BUTTON, true, 24);
    quit->position(470, 340);
    send->position(quit->x + quit->width + 25, quit->y);
    elements.push_back(send);
    elements.push_back(quit);
    
    send->action = std::bind(&Feedback::submit, this);
    quit->action = std::bind(&Feedback::back, this);
    
    TextElement* response = new TextElement("If you need to send more detailed feedback, please email us at fight@fortheusers.org", 20, NULL, false, 360);
    response->position(860, 20);
    elements.push_back(response);
}

void Feedback::submit()
{
    CURL *curl;
    CURLcode res;

#if defined(__WIIU__)
    const char* userKey = "wiiu_user";
#else
    const char* userKey = "switch_user";
#endif

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://switchbru.com/appstore/feedback");
        std::string fields = std::string("name=") + userKey + "&package=" + package->pkg_name + "&message=" + this->message;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields.c_str());

        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    // close this window
    this->back();
}

void Feedback::back()
{
    MainDisplay::subscreen = NULL;  // todo: clean up memory
}
