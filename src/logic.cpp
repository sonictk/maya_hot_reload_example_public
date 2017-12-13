#include "logic.h"
#include <ssmath/common_math.h>


globalVar int kMySize;


void foo(int *thing, int size)
{
	thing[size - 1] = 90000;
	fprintf(stderr, "%d\n", thing[size - 1]);
}


Shared
{
	DLLExport Vec3 getValue(Vec3 &v, float factor)
	{
		// NOTE: (sonictk) Business logic goes here

		// NOTE: (yliangsiew) Random code to test if heap allocation works
		kMySize = 10;
		int *test = (int *)malloc(sizeof(int) * kMySize);
		foo(test, kMySize);

		Vec3 result = vec3();

		result.x = v.x * 6;
		result.y = v.y * 4;
		result.z = v.z * 15;

		result = lerp(v, factor, result);

		return result;
	}
}
