#if defined(NOGUI)
	#include "console/Menu.hpp"
	#include "console/Input.hpp"
#else
	#include "gui/MainDisplay.hpp"
#endif

#include "libs/get/src/Utils.hpp"
#include "libs/get/src/Get.hpp"

#if defined(__WIIU__)
#define DEFAULT_REPO "http://wiiubru.com/appstore"
#else
#define DEFAULT_REPO "http://switchbru.com/appstore"
#endif

#if defined(SWITCH)
	#include <switch.h>
#endif

#if defined(__WIIU__)
	#include <unistd.h>


//remove when the wiiu starts working
#include <whb/log.h>
#include <whb/log_udp.h>
#include <sys/iosupport.h>
static devoptab_t dotab_stdout;
static ssize_t wiiu_log_write (struct _reent *r, void *fd, const char *ptr, size_t len) {
	WHBLogPrintf("%*.*s", len, len, ptr);
	return len;
}

#endif

int main(int argc, char *argv[])
{
//	consoleDebugInit(debugDevice_SVC);
//	stdout = stderr; // for yuzu

#if defined(__WIIU__)

//remove when the wiiu starts working
WHBLogUdpInit();
memset(&dotab_stdout, 0, sizeof(devoptab_t));
dotab_stdout.name = "stdout_udp";
dotab_stdout.write_r = &wiiu_log_write;
devoptab_list[STD_OUT] = &dotab_stdout;
devoptab_list[STD_ERR] = &dotab_stdout;

	chdir("fs:/vol/external01/wiiu/apps/appstore");
#endif
    init_networking();

    // create main get object
    Get* get = new Get("./.get/", DEFAULT_REPO);

#if defined(NOGUI)
	// if NOGUI variable defined, use the console's main method
	int console_main(Get*);
	return console_main(get);
#else

	// initialize main title screen
	MainDisplay* display = new MainDisplay(get);

	// the main inuput handler
	InputEvents* events = new InputEvents();

	bool running = true;
	while (running)
	{
        bool atLeastOneNewEvent = false;
        bool viewChanged = false;

        int frameStart = SDL_GetTicks();

		// get any new input events
		while(events->update())
        {
            // process the inputs of the supplied event
            viewChanged |= display->process(events);
            atLeastOneNewEvent = true;
        }

        // one more event update if nothing changed or there were no previous events seen
        // needed to non-input related processing that might update the screen to take place
        if ((!atLeastOneNewEvent && !viewChanged) || display->showingSplash)
        {
            events->update();
            viewChanged |= display->process(events);
        }

		// draw the display if we processed an event or the view
        if (viewChanged)
            display->render(NULL);
        else
        {
            // delay for the remainder of the frame to keep up to 60fps
            // (we only do this if we didn't draw to not waste energy
            // if we did draw, then proceed immediately without waiting for smoother progress bars / scrolling)
            int delayTime = (SDL_GetTicks() - frameStart);
            if (delayTime < 0)
                delayTime = 0;
            if (delayTime < 16)
                SDL_Delay(16 - delayTime);
        }

		// quit on enter/start
        if (events->held(START_BUTTON))
            running = false;
	}

    quit();

	return 0;
#endif
}
