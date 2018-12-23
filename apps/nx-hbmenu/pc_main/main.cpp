#include <SFML/Graphics.hpp>
#include <string.h>
#include <math.h>

extern "C" {

#include "../common/common.h"

}

color_t pixels[720][1280];

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Test");
    window.setFramerateLimit(60);

    menuStartupPath();
    assetsInit();
    themeStartup(THEME_PRESET_LIGHT);
    textInit();
    fontInitialize();
    netloaderInit();
    workerInit();
    menuStartup();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                break;
            }
        }

        memset(pixels, 237, sizeof(pixels));

        if (!uiUpdate()) break;
        menuLoop();
        window.clear();

        sf::Image image;
        image.create(1280, 720, (const unsigned char*)pixels);

        sf::Texture texture;
        texture.loadFromImage(image);

        sf::Sprite sprite;
        sprite.setTexture(texture);

        window.draw(sprite);
        window.display();
    }

    netloaderSignalExit();
    workerExit();
    netloaderExit();
    fontExit();
    assetsExit();

    return 0;
}

extern "C" bool menuUpdate(void) {
    //This is implemented here due to the hid code.
    menu_s* menu = menuGetCurrent();

    static int esc_state = 0;
    int new_esc_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
    static int return_state = 0;
    int new_return_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Return);
    static int y_state = 0;
    int new_y_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Y);
    static int t_state = 0;
    int new_t_state = sf::Keyboard::isKeyPressed(sf::Keyboard::T);

    if(!new_y_state && y_state)
    {
        launchMenuNetloaderTask();

    }

    if (!new_esc_state && esc_state)
    {
        launchMenuBackTask();
    }
    else if(!new_t_state && t_state){
        themeMenuStartup();
    }
    else if (!new_return_state && return_state)
    {
        menuHandleAButton();
    }
    else if (menu->nEntries > 0)
    {
        int move = 0;

        static int left_state = 0;
        int new_left_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
        if (!new_left_state && left_state)
            move--;
        left_state = new_left_state;

        static int right_state = 0;
        int new_right_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
        if (!new_right_state && right_state)
            move++;
        right_state = new_right_state;

        int newEntry = menu->curEntry + move;
        if (newEntry < 0) newEntry = 0;
        if (newEntry >= menu->nEntries) newEntry = menu->nEntries-1;
        menu->curEntry = newEntry;
    }

    esc_state = new_esc_state;
    return_state = new_return_state;
    y_state = new_y_state;
    t_state = new_t_state;

    return 0;
}
