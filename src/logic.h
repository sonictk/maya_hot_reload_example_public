#ifndef LOGIC_H
#define LOGIC_H

#include <limits.h>

#define PLATFORM_LEAN
#include <ssmath/platform.h>
#include <ssmath/vector_math.h>


enum DeformResult
{
	DeformResult_Failure = INT_MIN,
	DeformResult_Success = 0
};


Shared
{
	/// Simple example function
	DLLExport Vec3 getValue(Vec3 &v, float factor);
}


#endif /* LOGIC_H */
