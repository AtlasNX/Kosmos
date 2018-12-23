#include <switch.h>
#include "../common/common.h"

static bool psmInitialized;
static bool psmCacheInitialized;
static uint32_t psmCacheCharge;
static bool psmCacheIsCharging;

bool powerGetDetails(uint32_t *batteryCharge, bool *isCharging) {
    ChargerType charger = ChargerType_None;
    bool hwReadsSucceeded = false;
    bool use_cache = false;
    Result rc = 0;

    *isCharging = false;
    *batteryCharge = 0;

    if (psmInitialized) {
        if (psmCacheInitialized) {
            rc = psmWaitStateChangeEvent(0);

            if (R_FAILED(rc)) use_cache = true;
        }

        rc = psmGetBatteryChargePercentage(batteryCharge);
        hwReadsSucceeded = R_SUCCEEDED(rc);
        if (use_cache) {
            *isCharging = psmCacheIsCharging;
        }
        else {
            rc = psmGetChargerType(&charger);
            hwReadsSucceeded &= R_SUCCEEDED(rc);
            *isCharging = (charger > ChargerType_None);
        }

        psmCacheCharge = *batteryCharge;
        psmCacheIsCharging = *isCharging;
        psmCacheInitialized = true;
    }

    return hwReadsSucceeded;
}

void powerInit(void) {
    uint32_t charge=0;
    bool isCharging=0;

    psmCacheInitialized = false;
    psmCacheCharge = 0;
    psmCacheIsCharging = false;

    if (!psmInitialized) {
        Result rc = psmInitialize();
        if (R_SUCCEEDED(rc)) {
            rc = psmBindStateChangeEvent(1, 1, 1);

            if (R_FAILED(rc)) psmExit();
            if (R_SUCCEEDED(rc)) {
                psmInitialized = true;
                powerGetDetails(&charge, &isCharging);//Init the cache.
            }
        }
    }
}

void powerExit(void) {
    if (psmInitialized) {
        psmExit();
        psmInitialized = false;
        psmCacheInitialized = false;
    }
}
