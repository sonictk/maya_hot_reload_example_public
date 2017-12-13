/**
 * @brief  Simple library for basic math vector types.
 */
#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include "intrinsics_math.h"
#include "common_math.h"


enum RotationOrder {
	kXYZ,
	kYZX,
	kZXY,
	kXZY,
	kYXZ,
	kZYX
};


/// Simple 2D vector for math operations.
union Vec2
{
	struct {
		float x, y;
	};
	struct {
		float u, v;
	};
	float e[2];

	inline float &operator[](int index) {
		return e[index];
	}
};

inline Vec2 vec2(float x, float y)
{
	Vec2 result;
	result.x = x;
	result.y = y;
	return result;
}

inline Vec2 vec2(int x, int y)
{
	Vec2 result = {{(float)x, (float)y}};
	return result;
}

inline Vec2 vec2()
{
	return vec2(0, 0);
}

inline Vec2 operator*(const float factor, const Vec2 &v)
{
	Vec2 result;
	result.x = factor * v.x;
	result.y = factor * v.y;
	return result;
}

inline Vec2 operator*(const Vec2 &v, const float factor)
{
	Vec2 result = factor * v;
	return result;
}

inline Vec2 &operator*=(Vec2 &v, const float factor)
{
	v = v * factor;
	return v;
}

inline Vec2 operator/(const Vec2 &v, const float factor)
{
	Vec2 result;
	result.x = v.x / factor;
	result.y = v.y / factor;
	return result;
}

inline Vec2 operator/(const float factor, const Vec2 &v)
{
	return v / factor;
}

inline Vec2 &operator/=(Vec2 &v, const float factor)
{
	v = v / factor;
	return v;
}

inline Vec2 operator-(const Vec2 &v)
{
	Vec2 result;
	result.x = -v.x;
	result.y = -v.y;
	return result;
}

inline Vec2 operator+(const Vec2 &v1, const Vec2 &v2)
{
	Vec2 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	return result;
}

inline Vec2 &operator+=(Vec2 &v1, const Vec2 &v2)
{
	v1 = v1 + v2;
	return v1;
}

inline Vec2 operator-(const Vec2 &v1, const Vec2 &v2)
{
	Vec2 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	return result;
}

inline Vec2 &operator-=(Vec2 &v1, const Vec2 &v2)
{
	v1 = v1 - v2;
	return v1;
}

inline bool operator==(const Vec2 &v1, const Vec2 &v2)
{
	if (areFloatsEqual(v1.x, v2.x) && areFloatsEqual(v1.y, v2.y)) {
		return true;
	} else {
		return false;
	}
}

inline bool operator!=(const Vec2 &v1, const Vec2 &v2)
{
	return !(v1 == v2);
}

inline float innerProduct(const Vec2 &v1, const Vec2 &v2)
{
	float result = (v1.x * v2.x) + (v1.y * v2.y);
	return result;
}

inline float length(const Vec2 &v)
{
	float result = squareRoot(innerProduct(v, v));
	return result;
}

inline Vec2 normalize(const Vec2 &v)
{
	Vec2 result = v * (1.0f / length(v));
	return result;
}

inline Vec2 lerp(Vec2 &v1, float t, Vec2 &v2)
{
	Vec2 result = ((1.0f - t) * v1) + (t * v2);
	return(result);
}


/// Simple 3D vector for math operations.
union Vec3
{
	struct {
		float x, y, z;
	};
	struct {
		float r, g, b;
	};
	struct {
		float u, v, w;
	};
	struct {
		Vec2 xy;
		float _ignored0;
	};
	struct {
		float _ignored1;
		Vec2 yz;
	};
	float e[3];

	inline float &operator[](int index) {
		return e[index];
	}
};

inline Vec3 vec3(float x, float y, float z)
{
	Vec3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

inline Vec3 vec3(int x, int y, int z)
{
	Vec3 result = {{(float)x, (float)y, (float)z}};
	return result;
}

inline Vec3 vec3()
{
	return vec3(0, 0, 0);
}

inline Vec3 operator*(const float factor, const Vec3 v)
{
	Vec3 result;
	result.x = factor * v.x;
	result.y = factor * v.y;
	result.z = factor * v.z;
	return result;
}

inline Vec3 operator*(const Vec3 &v, const float factor)
{
	Vec3 result = factor * v;
	return result;
}

inline Vec3 &operator*=(Vec3 &v, const float factor)
{
	v = v * factor;
	return v;
}

inline Vec3 operator/(const Vec3 &v, const float factor)
{
	Vec3 result;
	result.x = v.x / factor;
	result.y = v.y / factor;
	result.z = v.z / factor;
	return result;
}

inline Vec3 operator/(const float factor, const Vec3 &v)
{
	return v / factor;
}

inline Vec3 &operator/=(Vec3 &v, const float factor)
{
	v = v / factor;
	return v;
}

inline Vec3 operator-(const Vec3 &v)
{
	Vec3 result;
	result.x = -v.x;
	result.y = -v.y;
	result.z = -v.z;
	return result;
}

inline Vec3 operator+(const Vec3 &v1, const Vec3 &v2)
{
	Vec3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

inline Vec3 &operator+=(Vec3 &v1, const Vec3 &v2)
{
	v1 = v1 + v2;
	return v1;
}

inline Vec3 operator-(const Vec3 &v1, const Vec3 &v2)
{
	Vec3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

inline Vec3 &operator-=(Vec3 &v1, const Vec3 &v2)
{
	v1 = v1 - v2;
	return v1;
}

inline bool operator==(const Vec3 &v1, const Vec3 &v2)
{
	if (areFloatsEqual(v1.x, v2.x) && areFloatsEqual(v1.y, v2.y) && areFloatsEqual(v1.z, v2.z)) {
		return true;
	} else {
		return false;
	}
}

inline bool operator!=(const Vec3 &v1, const Vec3 &v2)
{
	return !(v1 == v2);
}

#if INSTRSET >= 2 // NOTE: (sonictk) Require SSE2 support for these intrinsics
#include "instrset.h"

inline __m128 loadVec3(const Vec3 &vec)
{
	// NOTE: (sonictk) Referenced from http://fastcpp.blogspot.com/2011/03/loading-3d-vector-into-sse-register.html
	// Load x, y with a 64 bit integer load (result: 00YX), upper 64 bits are zeroed
	__m128i xy = _mm_loadl_epi64((const __m128i*)&vec);
	// NOTE: (sonictk) Now load the z element using a 32 bit float load (000Z)
	__m128 z = _mm_load_ss(&vec.z);
	// NOTE: (sonictk) Now cast the __m128i register into a __m128 one (0ZYX)
	return _mm_movelh_ps(_mm_castsi128_ps(xy), z);

	// NOTE: (sonictk) This alternative is for non 8byte alignment (64bit movq is slow when address of data is not 8byte aligned)
	// __m128 x = _mm_load_ss(&vec.x);
	// __m128 y = _mm_load_ss(&vec.y);
	// __m128 z = _mm_load_ss(&vec.z);
	// __m128 xy = _mm_movelh_ps(x, y);
	// return _mm_shuffle_ps(xy, z, _MM_SHUFFLE(2, 0, 2, 0));
}

inline Vec3 crossProduct(const Vec3 &v1, const Vec3 &v2)
{
	// NOTE: (sonictk) Referenced from http://fastcpp.blogspot.com/2011/04/vector-cross-product-using-sse-code.html
	__m128 sseV1 = _mm_set_ps(v1.x, v1.y, v1.z, 0);
	__m128 sseV2 = _mm_set_ps(v2.x, v2.y, v2.z, 0);
	__m128 sseV = _mm_sub_ps(
		_mm_mul_ps(sseV2, _mm_shuffle_ps(sseV1, sseV1, _MM_SHUFFLE(3, 0, 2, 1))),
		_mm_mul_ps(sseV1, _mm_shuffle_ps(sseV2, sseV2, _MM_SHUFFLE(3, 0, 2, 1)))
		);
	__m128 sseResult = _mm_shuffle_ps(sseV, sseV, _MM_SHUFFLE(3, 0, 2, 1 ));

	float fResult[4];
	_mm_store_ps(fResult, sseResult);

	Vec3 result;
	result.x = fResult[0];
	result.y = fResult[1];
	result.z = fResult[2];

	return result;
}

#else
inline Vec3 crossProduct(const Vec3 &v1, const Vec3 &v2)
{
	Vec3 result;
	result.x = (v1.y * v2.z) - (v1.z * v2.y);
	result.y = (v1.z * v2.x) - (v1.x * v2.z);
	result.z = (v1.x * v2.y) - (v1.y * v2.x);
	return result;
}

#endif // INSTRSET

#if INSTRSET >=3 // NOTE: (sonictk) Require SSE 4.1 support for these intrinsics
#include "smmintrin.h"

inline float innerProduct(const Vec3 &v1, const Vec3 &v2)
{
	//__m128 sseV1 = _mm_set_ps(v1.x, v1.y, v1.z, 0);
	//__m128 sseV2 = _mm_set_ps(v2.x, v2.y, v2.z, 0);
	//return _mm_cvtss_f32(_mm_dp_ps(sseV1, sseV2, 0xE1)); // NOTE: (sonictk) This is 1110 0001 in binary

	__m128 sseV1 = loadVec3(v1); // NOTE: (sonictk) loadVec3 loads it in 0ZYX order
	__m128 sseV2 = loadVec3(v2);
	return _mm_cvtss_f32(_mm_dp_ps(sseV1, sseV2, 0x71)); // NOTE: (sonictk) This is 0111 0001 in binary
}

#else
inline float innerProduct(const Vec3 &v1, const Vec3 &v2)
{
	float result = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	return result;
}

#endif	// INSTRSET


inline float length(const Vec3 &v)
{
	float result = squareRoot(innerProduct(v, v));
	return result;
}

inline Vec3 normalize(const Vec3 &v)
{
	Vec3 result = v * (1.0f / length(v));
	return result;
}

inline Vec3 lerp(Vec3 &v1, float t, Vec3 &v2)
{
	Vec3 result = ((1.0f - t) * v1) + (t * v2);
	return(result);
}


/// Simple 4D vector for math operations.
union Vec4
{
	struct {
		union {
			Vec3 xyz;
			struct {
				float x, y, z;
			};
		};
		float w;
	};
	struct {
		union {
			Vec3 rgb;
			struct {
				float r, g, b;
			};
		};
		float a;
	};
	struct {
		Vec2 xy;
		float _ignored0, _ignored1;
	};
	struct {
		float _ignored2;
		Vec2 yz;
		float _ignored3;

	};
	struct {
		float _ignored4;
		float _ignored5;
		Vec2 zw;
	};
	float e[4];

	inline float &operator[](int index) {
		return e[index];
	}
};

inline Vec4 vec4(float x, float y, float z, float w)
{
	Vec4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
}

inline Vec4 vec4(int x, int y, int z, int w)
{
	Vec4 result = {{(float)x, (float)y, (float)z, (float)w}};
	return result;
}

inline Vec4 vec4()
{
	return vec4(0, 0, 0, 0);
}

inline Vec4 vec4(Vec3 vec, float w)
{
	return vec4(vec.x, vec.y, vec.z, w);
}

inline Vec4 vec4(Vec3 vec)
{
	return vec4(vec, 0);
}

inline Vec4 operator*(const float factor, const Vec4 v)
{
	Vec4 result;
	result.x = factor * v.x;
	result.y = factor * v.y;
	result.z = factor * v.z;
	result.w = factor * v.w;
	return result;
}

inline Vec4 operator*(const Vec4 &v, const float factor)
{
	Vec4 result = factor * v;
	return result;
}

inline Vec4 &operator*=(Vec4 &v, const float factor)
{
	v = v * factor;
	return v;
}

inline Vec4 operator/(const Vec4 &v, const float factor)
{
	Vec4 result;
	result.x = v.x / factor;
	result.y = v.y / factor;
	result.z = v.z / factor;
	result.w = v.w / factor;
	return result;
}

inline Vec4 operator/(const float factor, const Vec4 &v)
{
	return v / factor;
}

inline Vec4 &operator/=(Vec4 &v, const float factor)
{
	v = v / factor;
	return v;
}

inline Vec4 operator-(const Vec4 &v)
{
	Vec4 result;
	result.x = -v.x;
	result.y = -v.y;
	result.z = -v.z;
	result.w = -v.w;
	return result;
}

inline Vec4 operator+(const Vec4 &v1, const Vec4 &v2)
{
	Vec4 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	result.w = v1.w + v2.w;
	return result;
}

inline Vec4 &operator+=(Vec4 &v1, const Vec4 &v2)
{
	v1 = v1 + v2;
	return v1;
}

inline Vec4 operator-(const Vec4 &v1, const Vec4 &v2)
{
	Vec4 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	result.w = v1.w - v2.w;
	return result;
}

inline Vec4 &operator-=(Vec4 &v1, const Vec4 &v2)
{
	v1 = v1 - v2;
	return v1;
}

inline bool operator==(const Vec4 &v1, const Vec4 &v2)
{
	if (areFloatsEqual(v1.x, v2.x)
		&& areFloatsEqual(v1.y, v2.y)
		&& areFloatsEqual(v1.z, v2.z)
		&& areFloatsEqual(v1.w, v2.w)) {
		return true;
	} else {
		return false;
	}
}

inline bool operator!=(const Vec4 &v1, const Vec4 &v2)
{
	return !(v1 == v2);
}

inline float innerProduct(const Vec4 &v1, const Vec4 &v2)
{
	float result = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
	return result;
}

inline Vec4 lerp(Vec4 &v1, float t, Vec4 &v2)
{
	Vec4 result = ((1.0f - t) * v1) + (t * v2);
	return(result);
}


// NOTE: (sonictk) This expects the values to be specified in **degrees**.
typedef Vec3 EulerRotation;

typedef Vec4 Quaternion;
typedef Quaternion Quat;


inline Vec3 rotateBy(Vec3 v, Quat rotation)
{
	Vec3 axis = vec3(rotation.x, rotation.y, rotation.z);
	float scalar = rotation.w;

	// NOTE: (sonictk) Derived from Rodrigues' rotation formula:
	// (https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula)
	// Implementation from: https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
	Vec3 result =
		(2.0f * innerProduct(axis, v) * axis) +
		(((scalar * scalar) - innerProduct(axis, axis)) * v) +
		(2.0f * scalar * crossProduct(axis, v));

	return result;
}


#endif /* VECTOR_MATH_H */
