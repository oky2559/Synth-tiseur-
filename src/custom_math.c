#include "custom_math.h"

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
