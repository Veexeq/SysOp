#include "integrator.h"

double function(double x) {
    return 4.0 / (x * x + 1.0);
}

double rectangular_integration(double start, double end, double dx) {
    double result = 0.0;
    
    double x = start;
    while (x < end) {
        result += function(x) * dx;
        x += dx;
    }

    return result;
}
