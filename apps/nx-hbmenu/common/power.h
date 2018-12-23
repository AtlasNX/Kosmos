#pragma once
#include "common.h"

void powerInit(void);

bool powerGetDetails(uint32_t *batteryCharge, bool *isCharging);

void powerExit(void);