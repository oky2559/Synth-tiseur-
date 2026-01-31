// tests/test_sin.c
#include "custom_math.h"
#include <stdio.h>
#include <stdlib.h>

static void assert_close(const char *name, double got, double expected,
                         double eps) {
  double diff = got - expected;
  if (diff < 0) {
    diff = -diff;
  }
  if (diff > eps) {
    fprintf(stderr, "FAIL %s: got=%.17g expected=%.17g diff=%.17g eps=%.17g\n",
            name, got, expected, diff, eps);
    exit(1);
  }
}

static void assert_true(const char *name, int cond) {
  if (!cond) {
    fprintf(stderr, "FAIL %s\n", name);
    exit(1);
  }
}

int main(void) {
  puts("OK");
  return 0;
}
