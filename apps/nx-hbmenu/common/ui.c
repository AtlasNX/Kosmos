#include "common.h"

static bool s_shouldExit;

void uiExitLoop(void) {
    s_shouldExit = true;
}

bool menuUpdate(void);

bool uiUpdate(void) {
    bool exitflag=0;

    exitflag = !menuUpdate();
    if (!exitflag) return exitflag;

    return !s_shouldExit;
}
