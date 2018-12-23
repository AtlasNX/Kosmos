#include "math.h"

// Low precision, adapted from http://lab.polygonal.de/2007/07/18/fast-and-accurate-sinecosine-approximation/
float approxSin(float x) {
    float ret;

    // always wrap input angle to -PI..PI
    x = fmod(x, M_PI*2.0)-M_PI;

    // compute sine
    if (x<0)
        ret = 1.27323954 * x + .405284735 * x * x;
    else
        ret = 1.27323954 * x - 0.405284735 * x * x;

    return ret;
}