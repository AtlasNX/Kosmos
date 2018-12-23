#include "../common/common.h"

static const loaderFuncs_s* s_loader;

bool launchInit(void) {
    return 1;
}

void launchExit(void) {
    //s_loader->deinit();
}

const loaderFuncs_s* launchGetLoader(void) {
    return s_loader;
}

void launchMenuEntry(menuEntry_s* me) {
    
}
