#include "../common/common.h"

void powerInit(void) {

}

void powerExit(void) {

}

bool powerGetDetails(uint32_t *batteryCharge, bool *isCharging) {
    *isCharging = false;
    *batteryCharge = 100;
    return false;
}
