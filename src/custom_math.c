#include "custom_math.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

double taylor_sin(double x)
{
    while (x > PI)
        x -= 2.0 * PI;
    while (x < -PI)
        x += 2.0 * PI;

    double term = x;
    double result = x;
    double x2 = x * x;

    for (int n = 1; n <= 10; n++) {
        term *= -x2 / ((2 * n) * (2 * n + 1));
        result += term;
    }

    return result;
}

float generate_sample(int type, float phase) {
    switch (type) {
        case 0: return sinf(2.0f * PI * phase);
        case 1: return (sinf(2.0f * PI * phase) >= 0) ? 1.0f : -1.0f;
        case 2: return 2.0f * fabsf(2.0f * (phase - floorf(phase + 0.5f))) - 1.0f;
        case 3: return 2.0f * (phase - floorf(phase + 0.5f));
        default: return 0.0f;
    }
}
