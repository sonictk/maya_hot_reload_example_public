#ifndef COMMON_MATH_H
#define COMMON_MATH_H

#define PI 3.14159265359f
#define TAU 6.28318530717958647692f

#include <math.h>
#include <float.h>


/**
 * This function checks if the two given floating-point numbers are equal within a
 * given tolerance.
 *
 * @param a 		The first number.
 * @param b 		The second number.
 * @param epsilon 	The tolerance.
 *
 * @return 		``true`` if the numbers are equal, ``false`` otherwise.
 */
inline bool areFloatsEqual(float a, float b, float epsilon)
{
	if (fabs(a - b) < epsilon) {
		return true;
	} else {
		return false;
	}
}

inline bool areFloatsEqual(float a, float b)
{
	return areFloatsEqual(a, b, FLT_EPSILON);
}


/**
 * This function will return a random value between ``0`` and ``1``.
 *
 * @return		A normalized random value.
 */
inline float getNormalizedRandomValue()
{
	return float(rand()) / float(RAND_MAX);
}


// TODO: (sonictk) *Maybe* this can be templated
inline float lerp(float a, float b, float t)
{
	float result = ((1.0f - t) * a) + (t * b);

	return result;
}


#endif /* COMMON_MATH_H */
