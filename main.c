#include <stdio.h>
#include <complex.h>
#include <stdbool.h>

#include <png.h>

#define MAX_N 32L                                                               /* max number of iterations for quadratic map */

#define RES   32L                                                               /* square image resolution */

complex double quad_map(complex double c, long n)
{
    switch (n) {
        case 0:
            return c;
        default:
            return (quad_map(c, n-1)*quad_map(c, n-1) + c);
    }
}

bool Mandelbrot(double complex c)
{
    for (long n = 0; n < MAX_N; ++n) {
        if (cabs(quad_map(c, n)) > 2) {
            return false;
        }
    }

    return true;
}

complex double px_to_C(double x, double y) {
    complex double c_re =  (x/RES) - 2.0;
    complex double c_im = -(y/RES) + 2.0;
    complex double c = c_re + I*c_im;

    return c;
}

int main()
{


    return 0;
}