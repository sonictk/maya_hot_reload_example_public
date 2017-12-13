/**
 * @brief  	Simple library for basic matrix types. Intended to be used with
 * 			the simple vector types defined in ``vector_math.h``.
 */
#ifndef MATRIX_MATH_H
#define MATRIX_MATH_H

#include "vector_math.h"
#include <math.h>


/// A variable-size matrix data structure. These are assumed to be **row-major**.
// TODO: (sonictk) Test if this works and implement more math ops for it
template<class T, int rows, int columns>
struct MatX
{
	T e[rows][columns];

	inline T *operator[](int row) {
		float *ptr = e[row];
		return ptr;
	}

	inline const T operator()(int row, int col) const {
		return e[row][col];
	}
};


/// A simple 4x4 matrix data structure. These are assumed to be **row-major**.
struct Mat44
{
	float e[4][4];

	inline float *operator[](int row) {
		return e[row];
	}

	inline const float *operator[](int row) const {
		return e[row];
	}

	inline const float &operator()(int row, int col) const {
		return e[row][col];
	}

	inline float &operator()(int row, int col) {
		return e[row][col];
	}
};

inline Mat44 mat44(const float vals[4][4])
{
	Mat44 mat = {
		{{vals[0][0], vals[0][1], vals[0][2], vals[0][3]},
		 {vals[1][0], vals[0][1], vals[0][2], vals[0][3]},
		 {vals[2][0], vals[0][1], vals[0][2], vals[0][3]},
		 {vals[3][0], vals[0][1], vals[0][2], vals[0][3]}}
	};

	return mat;
}

inline Mat44 mat44(const int vals[4][4])
{
	Mat44 mat = {
		{{(float)vals[0][0], (float)vals[0][1], (float)vals[0][2], (float)vals[0][3]},
		 {(float)vals[1][0], (float)vals[0][1], (float)vals[0][2], (float)vals[0][3]},
		 {(float)vals[2][0], (float)vals[0][1], (float)vals[0][2], (float)vals[0][3]},
		 {(float)vals[3][0], (float)vals[0][1], (float)vals[0][2], (float)vals[0][3]}}
	};

	return mat;
}

inline Mat44 mat44()
{
	Mat44 mat = {
		{{1, 0, 0, 0},
		 {0, 1, 0, 0},
		 {0, 0, 1, 0},
		 {0, 0, 0, 1}}
	};

	return mat;
}

inline Mat44 identityMat44()
{
	return mat44();
}

inline void getValues(const Mat44 &mat, float *f, int size)
{
	int i = 0;
	while (i < size && i < 16) {
		for (int r=0; r <= 3; ++r) {
			for (int c=0; c <= 3; ++c) {
				f[i] = mat[r][c];
				++i;
			}
		}
	}
}

inline Mat44 operator+(const Mat44 &a, const Mat44 &b)
{
	Mat44 result = {
		{{a(0, 0) + b(0, 0), a(0, 1) + b(0, 1), a(0, 2) + b(0, 2), a(0, 3) + b(0, 3)},
		 {a(1, 0) + b(1, 0), a(1, 1) + b(1, 1), a(1, 2) + b(1, 2), a(1, 3) + b(1, 3)},
		 {a(2, 0) + b(2, 0), a(2, 1) + b(2, 1), a(2, 2) + b(2, 2), a(2, 3) + b(2, 3)},
		 {a(3, 0) + b(3, 0), a(3, 1) + b(3, 1), a(3, 2) + b(3, 2), a(3, 3) + b(3, 3)}}
	};

	return result;
}

inline Mat44 &operator+=(Mat44 &a, const Mat44 &b)
{
	a = a + b;
	return a;
}

inline Mat44 operator-(const Mat44 &a, const Mat44 &b)
{
	// TODO: (sonictk) Investigate optimizing this using SIMD
	Mat44 result = {
		{{a(0, 0) - b(0, 0), a(0, 1) - b(0, 1), a(0, 2) - b(0, 2), a(0, 3) - b(0, 3)},
		 {a(1, 0) - b(1, 0), a(1, 1) - b(1, 1), a(1, 2) - b(1, 2), a(1, 3) - b(1, 3)},
		 {a(2, 0) - b(2, 0), a(2, 1) - b(2, 1), a(2, 2) - b(2, 2), a(2, 3) - b(2, 3)},
		 {a(3, 0) - b(3, 0), a(3, 1) - b(3, 1), a(3, 2) - b(3, 2), a(3, 3) - b(3, 3)}}
	};

	return result;
}

inline Mat44 &operator-=(Mat44 &a, const Mat44 &b)
{
	a = a - b;
	return a;
}

inline Mat44 operator*(const Mat44 &mat, float factor)
{
	Mat44 result = {
		{{mat(0, 0) * factor, mat(0, 1) * factor, mat(0, 2) * factor, mat(0, 3) * factor},
		 {mat(1, 0) * factor, mat(1, 1) * factor, mat(1, 2) * factor, mat(1, 3) * factor},
		 {mat(2, 0) * factor, mat(2, 1) * factor, mat(2, 2) * factor, mat(2, 3) * factor},
		 {mat(3, 0) * factor, mat(3, 1) * factor, mat(3, 2) * factor, mat(3, 3) * factor}}
	};

	return result;
}

inline Mat44 operator*(const Mat44 &a, const Mat44 &b)
{
	// TODO: (sonictk) Investigate optimizing this using SIMD
	Mat44 result = {};
	for (int r=0; r <= 3; ++r) {
		for (int c=0; c <=3; ++c) {
			for (int i=0; i <= 3; ++i) {
				result[r][c] += a(r, i) * b(i, c);
			}
		}
	}

	return result;
}

inline Mat44 &operator*=(Mat44 &mat, float factor)
{
	mat = mat * factor;
	return mat;
}

inline Mat44 &operator*=(Mat44 &a, const Mat44 &b)
{
	a = a * b;
	return a;
}

inline Vec4 operator*(const Mat44 &mat, Vec4 vec)
{
	Vec4 result;

	result.x = (vec.x * mat[0][0]) + (vec.y * mat[0][1]) + (vec.z * mat[0][2]) + (vec.w * mat[0][3]);
	result.y = (vec.x * mat[1][0]) + (vec.y * mat[1][1]) + (vec.z * mat[1][2]) + (vec.w * mat[1][3]);
	result.z = (vec.x * mat[2][0]) + (vec.y * mat[2][1]) + (vec.z * mat[2][2]) + (vec.w * mat[2][3]);
	result.w = (vec.x * mat[3][0]) + (vec.y * mat[3][1]) + (vec.z * mat[3][2]) + (vec.w * mat[3][3]);

	return result;
}

inline Vec3 operator*(const Mat44 &mat, Vec3 vec)
{
	Vec3 result = (mat * vec4(vec, 1.0f)).xyz;
	return result;
}

inline Mat44 operator/(const Mat44 &mat, float factor)
{
	// TODO: (sonictk) Investigate optimizing this using SIMD
	Mat44 result = {
		{{mat(0, 0) / factor, mat(0, 1) / factor, mat(0, 2) / factor, mat(0, 3) / factor},
		 {mat(1, 0) / factor, mat(1, 1) / factor, mat(1, 2) / factor, mat(1, 3) / factor},
		 {mat(2, 0) / factor, mat(2, 1) / factor, mat(2, 2) / factor, mat(2, 3) / factor},
		 {mat(3, 0) / factor, mat(3, 1) / factor, mat(3, 2) / factor, mat(3, 3) / factor}}
	};

	return result;
}

inline Mat44 &operator/=(Mat44 &mat, float factor)
{
	mat = mat / factor;
	return mat;
}

inline bool operator==(const Mat44 &mat1, const Mat44 &mat2)
{
	for (int r=0; r <= 3; ++r) {
		for (int c=0; c <= 3; ++c) {
			if (!areFloatsEqual(mat1[r][c], mat2[r][c])) {
				return false;
			}
		}
	}
	return true;
}

inline bool operator!=(const Mat44 &mat1, const Mat44 &mat2)
{
	return !(mat1 == mat2);
}

inline Mat44 transpose(const Mat44 &mat)
{
	Mat44 result;
	for (int j=0; j <= 3; ++j) {
		for (int i=0; i <= 3; ++i) {
			result[j][i] = mat(i, j);
		}
	}

	return result;
}


/**
 * Translates the given matrix by the given ``translation``.
 * This is for **row-major** matrices (i.e. XYZ stored in the 4th/8th/12th indices).
 *
 * @param mat			The matrix to apply the translation to.
 * @param translation	The translation to apply.
 *
 * @return				The translated matrix.
 */
inline Mat44 translateBy(const Mat44 &mat, Vec3 translation)
{
	Mat44 result = mat;
	result[0][3] += translation.x;
	result[1][3] += translation.y;
	result[2][3] += translation.z;

	return result;
}

/**
 * Translates the given matrix by the given ``x``, ``y`` and ``z`` values.
 * This is for **row-major** matrices (i.e. XYZ stored in the 4th/8th/12th indices
 * for a matrix with indices starting from 1 and incrementing left-to-right).
 *
 * @param mat			The matrix to apply the translation to.
 * @param x			The translation to apply in the X-axis.
 * @param y			The translation to apply in the Y-axis.
 * @param z			The translation to apply in the Z-axis.
 *
 * @return				The translated matrix.
 */
inline Mat44 translateBy(const Mat44 &mat, float x, float y, float z)
{
	Mat44 result = mat;
	result[0][3] += x;
	result[1][3] += y;
	result[2][3] += z;

	return result;
}


inline Mat44 xRotation(float angle)
{
	float angleRad = angle * PI / 180.0f;
	float c = cosf(angleRad);
	float s = sinf(angleRad);

	Mat44 result = {
		{{1, 0, 0, 0},
		 {0, c, -s, 0},
		 {0, s, c, 0},
		 {0, 0, 0, 1}}
	};

	return result;
}

inline Mat44 yRotation(float angle)
{
	float angleRad = angle * PI / 180.0f;
	float c = cosf(angleRad);
	float s = sinf(angleRad);

	Mat44 result = {
		{{c, 0, s, 0},
		 {0, 1, 0, 0},
		 {-s, 0, c, 0},
		 {0, 0, 0, 1}}
	};

	return result;
}

inline Mat44 zRotation(float angle)
{
	float angleRad = angle * PI / 180.0f;
	float c = cosf(angleRad);
	float s = sinf(angleRad);

	Mat44 result = {
		{{c, -s, 0, 0},
		 {s, c, 0, 0},
		 {0, 0, 1, 0},
		 {0, 0, 0, 1}}
	};

	return result;
}


/**
 * Rotates the given transformation matrix by the specified axis-angle rotation.
 *
 * @param mat		The 4x4 transformation matrix to apply the rotation to.
 * @param axis		The axis in which the rotation should be applied around.
 * @param angle	The amount of rotation in **degrees** to apply.
 *
 * @return			The rotated transformation matrix.
 */
Mat44 rotateBy(const Mat44 &mat, Vec3 axis, float angle);

Mat44 rotateBy(const Mat44 &mat, Quat rotation);

Mat44 rotateBy(const Mat44 &mat, EulerRotation rotation, RotationOrder order);

Mat44 rotateBy(const Mat44 &mat, EulerRotation rotation);


inline Mat44 scaleBy(const Mat44 &mat, float x, float y, float z)
{
	Mat44 result;

	Mat44 scaleMatrix = mat44();
	scaleMatrix[0][0] = x;
	scaleMatrix[1][1] = y;
	scaleMatrix[2][2] = z;
	result = mat * scaleMatrix;

	return result;
}

inline Mat44 scaleBy(const Mat44 &mat, Vec3 scale)
{
	return scaleBy(mat, scale.x, scale.y, scale.z);
}

inline Mat44 scaleBy(Mat44 mat, float factor)
{
	return scaleBy(mat, factor, factor, factor);
}


/**
 * Calculate the determinant of a 4x4 matrix. This should be used instead of the
 * other version when the size of the matrix is 4x4, as it is faster.
 *
 * @param mat	A 4x4 matrix.
 *
 * @return		The determinant.
 */
float determinant(const Mat44 &mat);


/**
 * Calculate the determinant of a variable-sized square matrix.
 *
 * @param mat			Pointer to a pointer of an arbitrary square matrix.
 * @param dimension	The dimension of the square matrix.
 *
 * @return				The determinant.
 */
float determinant(float **mat, int dimension);


/**
 * This finds the matrix of cofactors for the given 4x4 matrix.
 *
 * @param mat	A 4x4 matrix.
 *
 * @return		The matrix of cofactors.
 */
Mat44 cofactor(const Mat44 &mat);


/**
 * This finds the adjugate/adjoint for the given 4x4 matrix.
 *
 * @param mat	A 4x4 matrix.
 *
 * @return		The adjugate/adjoint matrix.
 */
Mat44 adjugate(const Mat44 &mat);


/**
 * This finds the inverse of the given 4x4 matrix.
 *
 * @param inMat 	The 4x4 matrix to find the inverse of.
 * @param outMat	The 4x4 matrix that will have the result written to.
 *
 * @return 		``0`` on success, a negative value if the inverse does not
 * 				exist or an error occurred.
 */
int inverse(const Mat44 &inMat, Mat44 &outMat);


#endif /* MATRIX_MATH_H */
