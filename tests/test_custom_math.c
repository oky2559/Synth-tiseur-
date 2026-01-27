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
  const double EPS = 1e-10;

  assert_close("sin(0)=0", taylor_sin(0.0), 0.0, EPS);
  assert_close("sin(pi/6)=0.5", taylor_sin(PI / 6.0), 0.5, EPS);
  assert_close("sin(pi/2)=1", taylor_sin(PI / 2.0), 1.0, EPS);
  assert_close("sin(-pi/2)=-1", taylor_sin(-PI / 2.0), -1.0, EPS);
  assert_close("sin(pi)=0", taylor_sin(PI), 0.0,
               5e-10);

  assert_close("periodicity(+2pi)", taylor_sin(0.37 + 2.0 * PI), taylor_sin(0.37), EPS);
  assert_close("periodicity(-2pi)", taylor_sin(-1.2 - 2.0 * PI),
               taylor_sin(-1.2), EPS);

  {
    double x = 1.234;
    assert_close("oddness", taylor_sin(-x), -taylor_sin(x), EPS);
  }

  {
    double xs[] = {-10.0, -3.0, -1.0, -0.1, 0.1, 1.0, 2.7, 10.0, 123.456};
    for (size_t i = 0; i < sizeof(xs) / sizeof(xs[0]); i++) {
      double s = taylor_sin(xs[i]);
      assert_true("range |sin(x)|<=1", s <= 1.0 + 1e-12 && s >= -1.0 - 1e-12);
    }
  }

  puts("OK");
  return 0;
}
