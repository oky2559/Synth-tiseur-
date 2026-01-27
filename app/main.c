#include <stdio.h>
#include <stdlib.h>

#include "custom_math.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <angle_in_radians>\n", argv[0]);
    return 1;
  }

  double angle = atof(argv[1]);
  double result = taylor_sin(angle);

  printf("sin(%f) ≈ %f\n", angle, result);
  return 0;
}
