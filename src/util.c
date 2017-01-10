#include <stdlib.h>

#include "util.h"

float32 Rand(float32 min, float32 max) {
    float32 r = ((float)rand() / (float)(RAND_MAX));
    return (max - min) * r + min;
}