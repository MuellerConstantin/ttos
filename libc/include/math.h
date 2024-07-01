#ifndef _MATH_H
#define _MATH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Computes the smallest integer value not less than `x`.
 * 
 * @param x The value to compute the ceiling of.
 * @return The smallest integer value not less than `x`.
 */
double ceil(double x);

#ifdef __cplusplus
}
#endif

#endif // _MATH_H
