#ifndef INTRINSICS_MATH_H
#define INTRINSICS_MATH_H

#include "instrset.h"

#if INSTRSET >= 2 // NOTE: (sonictk) Require SSE2 support for these intrinsics
inline float squareRoot(const float val)
{
	float result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(val)));
	return result;
}

#else // NOTE: (sonictk) Fallback implementation
#include <math.h>

inline float squareRoot(const float val)
{
	return sqrt(val);
}
#endif /* INSTRSET */

#endif /* INTRINSICS_MATH_H */
