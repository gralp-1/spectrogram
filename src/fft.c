#include <assert.h>
#include <complex.h>
#include <stdlib.h>
void fft(float in[], size_t stride, double complex out[], size_t n) {
  assert(n > 0);

  if (n == 1) {
    out[0] = in[0];
    return;
  }

  fft(in, stride * 2, out, n / 2);
  fft(in + stride, stride * 2, out + n / 2, n / 2);

  for (size_t k = 0; k < n / 2; ++k) {
    double t = (double)k / n;
    double complex v = cexp(-2 * I * PI * t) * out[k + n / 2];
    double complex e = out[k];
    out[k] = e + v;
    out[k + n / 2] = e - v;
  }
}
