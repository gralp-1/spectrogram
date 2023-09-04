#include <complex.h>

#include <assert.h>
#include <complex.h>
#include <math.h>
#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
typedef double complex cplx;
char *format_cplx(cplx in) {
  float real = creal(in);
  float imag = cimag(in);
  char *out = malloc(sizeof(char) * 20);
  sprintf(out, "%5.2f %5.2f i", real, imag);
  return out;
}
void print_complex_array(float *ptr, size_t length) {
  // for statement to print values using array
  size_t i = 0;
  printf("{");
  for (; i < length; ++i)
    printf("%s ", format_cplx(ptr[i]));
  printf("}\n");
}
void print_array(float *ptr, size_t length) {
  // for statement to print values using array
  size_t i = 0;
  printf("{");
  for (; i < length; ++i)
    printf("%f ", ptr[i]);
  printf("}\n");
}
