#include "oscillators.h"
#include "common.h" // Pour PI
#include <math.h>

float generate_sample(int type, float phase) {
    switch (type) {
        case 0: return sinf(2.0f * PI * phase);
        case 1: return (sinf(2.0f * PI * phase) >= 0) ? 1.0f : -1.0f;
        case 2: return 2.0f * fabsf(2.0f * (phase - floorf(phase + 0.5f))) - 1.0f;
        case 3: return 2.0f * (phase - floorf(phase + 0.5f));
        default: return 0.0f;
    }
}