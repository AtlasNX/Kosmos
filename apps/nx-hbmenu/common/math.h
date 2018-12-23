#pragma once

#include "common.h"

float approxSin(float x);

static inline float clamp(float x, float min, float max) {
    return fmin(fmax(x, min), max);
}