#include "matrix_math.h"
#include "common_math.h"
#include <cstdlib>
#include <math.h>

using std::malloc;
using std::free;


Mat44 rotateBy(const Mat44 &mat, Vec3 axis, float angle)
{
	// NOTE: (sonictk) Implementation from:
	// http://www.opengl-tutorial.org/assets/faq_quaternions/index.html#Q38
	// Basically it's rotation of matrix by a quaternion
	Mat44 rotMatrix;

	float angleRad = angle * PI / 180.0f;
	float rcos = cosf(angleRad);
	float rsin = sinf(angleRad);

	float u = axis.x;
	float v = axis.y;
	float w = axis.z;

	// NOTE: (sonictk) This is the upper 3x3 part of the rot matrix
	rotMatrix[0][0] = rcos + u*u*(1-rcos);
	rotMatrix[1][0] = w * rsin + v*u*(1-rcos);
	rotMatrix[2][0] = -v * rsin + w*u*(1-rcos);
	rotMatrix[0][1] = -w * rsin + u*v*(1-rcos);
	rotMatrix[1][1] = rcos + v*v*(1-rcos);
	rotMatrix[2][1] = u * rsin + w*v*(1-rcos);
	rotMatrix[0][2] = v * rsin + u*w*(1-rcos);
	rotMatrix[1][2] = -u * rsin + v*w*(1-rcos);
	rotMatrix[2][2] = rcos + w*w*(1-rcos);

	// NOTE: (sonictk) Set the rest of the rot matrix to identity
	rotMatrix[0][3] = rotMatrix[1][3] = rotMatrix[2][3] = rotMatrix[3][0] = rotMatrix[3][1] = rotMatrix[3][2] = 0;
	rotMatrix[3][3] = 1.0f;

	return mat * rotMatrix;
}

Mat44 rotateBy(const Mat44 &mat, Quat rotation)
{
	return rotateBy(mat, vec3(rotation.x, rotation.y, rotation.z), rotation.w);
}

Mat44 rotateBy(const Mat44 &mat, EulerRotation rotation, RotationOrder order)
{
	// NOTE: (sonictk) Implementation from:
	// http://www.opengl-tutorial.org/assets/faq_quaternions/index.html#Q36
	Mat44 rotMat;

	float x = rotation.x * PI / 180.0f;
	float y = rotation.y * PI / 180.0f;;
	float z = rotation.z * PI / 180.0f;;

	// TODO: (sonictk) Finish the other rotation order implementations
	// NOTE: (sonictk) Basically, the rotation matrix is generated from ``X.Y.Z`` and
	// then simplified to get the optimized version seen below.
	switch(order) {
	case RotationOrder::kXZY:
		break;
	case RotationOrder::kYXZ:
		break;
	case RotationOrder::kYZX:
		break;
	case RotationOrder::kZXY:
		break;
	case RotationOrder::kZYX:
		break;
	case RotationOrder::kXYZ:
	default:
		float a = cosf(x);
		float b = sinf(x);
		float c = cosf(y);
		float d = sinf(y);
		float e = cosf(z);
		float f = sinf(z);

		float ad = a * d;
		float bd = b * d;

		rotMat[0][0] = c * e;
		rotMat[0][1] = -c * f;
		rotMat[0][2] = d;
		rotMat[1][0] = (bd * e) + (a * f);
		rotMat[1][1] = (-bd * f) + (a * e);
		rotMat[1][2] = -b * c;
		rotMat[2][0] = (-ad * e) + (b * f);
		rotMat[2][1] = (ad * f) + (b * e);
		rotMat[2][2] = a * c;

		rotMat[0][3] = rotMat[1][3] = rotMat[2][3] = rotMat[3][0] = rotMat[3][1] = rotMat[3][2] = 0;
		rotMat[3][3] = 1.0f;
	}

	return mat * rotMat;
}

Mat44 rotateBy(const Mat44 &mat, EulerRotation rotation)
{
	return rotateBy(mat, rotation, RotationOrder::kXYZ);
}


float determinant(const Mat44 &mat)
{
	/* NOTE: (sonictk) After expansion, the formula for a determinant of a 4x4
	   matrix is:
	   ```
	   + a.[f.(k.p - l.o) - g.(j.p - l.n) + h.(j.o - k.n)]
	   - b.[e.(k.p - l.o) - g.(i.p - l.m) + h.(i.o - k.m)]
	   + c.[e.(j.p - l.n) - f.(i.p - l.m) + h.(i.n - j.m)]
	   - d.[e.(j.o - k.n) - f.(i.o - k.m) + g.(i.n - j.m)]
	   ```
	   where the 4x4 matrix is composed as such:
	       0 1 2 3
	   0  |a b c d|
	   1  |e f g h|
	   2  |i k j l|
	   3  |m n o p|
	*/
	// TODO: (sonictk) Check if this is correct
	return
		+ (mat(0,0) * ((mat(1,1) * ((mat(2,1) * mat(3,3)) - (mat(2,3) * mat(3,2))))
					   - (mat(1,2) * (mat(2,2) * mat(3,3) - mat(2,3) * mat(3,1)))
					   + (mat(1,3) * (mat(2,2) * mat(3,2) - mat(2,1) * mat(3,1)))))
		- (mat(0,1) * ((mat(1,0) * ((mat(2,1) * mat(3,3)) - (mat(2,3) * mat(3,2))))
					   - (mat(1,2) * (mat(2,0) * mat(3,3) - mat(2,3) * mat(3,0)))
					   + (mat(1,3) * (mat(2,0) * mat(3,2) - mat(2,1) * mat(3,0)))))
		+ (mat(0,2) * ((mat(1,0) * ((mat(2,2) * mat(3,3)) - (mat(2,3) * mat(3,1))))
					   - (mat(1,1) * (mat(2,0) * mat(3,3) - mat(2,3) * mat(3,0)))
					   + (mat(1,3) * (mat(2,0) * mat(3,1) - mat(2,2) * mat(3,0)))))
		- (mat(0,3) * ((mat(1,0) * ((mat(2,2) * mat(3,2)) - (mat(2,1) * mat(3,1))))
					   - (mat(1,1) * (mat(2,0) * mat(3,2) - mat(2,1) * mat(3,0)))
					   + (mat(1,2) * (mat(2,0) * mat(3,1) - mat(2,2) * mat(3,0)))));
}


// * Referenced from **Paul Bourke's** implementation available at:
// * ``http://paulbourke.net/miscellaneous/determinant/determinant.c``
// TODO: (sonictk) clean this up and understand it and possibly optimize it
float determinant(float **mat, unsigned int dimension)
{
	unsigned int i, j, j1, j2 ;				// general loop and matrix subscripts
	float det = 0;
	float **m = NULL ;				// pointer to pointers to implement 2d square array

	switch (dimension) {
	case 0:
		return 0;
		break;
	case 1:
		return mat[0][0];
		break;
	case 2:	// the recursion series
		det = mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1];
		break;
	default:
		// for each column in sub-matrix
		for (j1 = 0 ; j1 < dimension ; j1++) {
			// get space for the pointer list
			m = (float **)malloc((dimension - 1) * sizeof(float *));
			for (i = 0 ; i < dimension-1 ; i++)
				m[i] = (float *)malloc((dimension - 1) * sizeof(float));
			//	  i[0][1][2][3]	 first malloc
			//  m -> +  +  +	 +	 space for 4 pointers
			//		|  |  |	 |			j  second malloc
			//		|  |  |	 +-> _ _ _ [0] pointers to
			//		|  |  +----> _ _ _ [1] and memory for
			//		|  +-------> _ a _ [2] 4 floats
			//		+----------> _ _ _ [3]
			//
			//					a[1][2]
			// build sub-matrix with minor elements excluded
			for (i=1; i < dimension; ++i) {
				j2 = 0;			   // start at first sum-matrix column position
									   // loop to copy source matrix less one column
				for (j=0; j < dimension; ++j) {
					if (j == j1) continue; // don't copy the minor column element

					m[i - 1][j2] = mat[i][j];	// copy source element into new sub-matrix
					// i-1 because new sub-matrix is one row
					// (and column) smaller with excluded minors
					j2++;	// move to next sub-matrix column position
				}
			}

			// sum x raised to y power recursively get determinant of
			// next sub-matrix which is now one row & column smaller
			det += pow(-1.0f, 1.0f + j1 + 1.0f) * mat[0][j1] * determinant(m, dimension - 1);

			for (i = 0 ; i < dimension-1 ; i++) {
				// free the storage allocated to to this minor's set of pointers
				free(m[i]);
			}
			// free the storage for the original pointer to pointer
			free(m);
		}

		break;
	}

	return det;
}

Mat44 cofactor(const Mat44 &mat)
{
	// TODO: (sonictk) Unit test this and make sure it works correctly
	Mat44 result;
	/* NOTE: (sonictk) Results were determined after expansion of the formula
	   for the cofactor of a 4x4
	   matrix where the 4x4 matrix is composed as such:

	       0 1 2 3
	   0  |a b c d|
	   1  |e f g h|
	   2  |i k j l|
	   3  |m n o p|
	*/
	float a = mat(0, 0);
	float b = mat(0, 1);
	float c = mat(0, 2);
	float d = mat(0, 3);

	float e = mat(1, 0);
	float f = mat(1, 1);
	float g = mat(1, 2);
	float h = mat(1, 3);

	float i = mat(2, 0);
	float j = mat(2, 1);
	float k = mat(2, 2);
	float l = mat(2, 3);

	float m = mat(3, 0);
	float n = mat(3, 1);
	float o = mat(3, 2);
	float p = mat(3, 3);

	result[0][0] =
		+ (f * ((k * p) - (l * o)))
		- (g * ((j * p) - (l * n)))
		+ (h * ((j * o) - (k * n)));
	result[0][1] =
		- (e * ((k * p) - (l * o)))
		+ (g * ((i * p) - (l * m)))
		- (h * ((i * o) - (k * m)));
	result[0][2] =
		+ (e * ((j * p) - (l * n)))
		- (f * ((i * p) - (l * m)))
		+ (h * ((i * n) - (j * m)));
	result[0][3] =
		- (e * ((j * o) - (k * n)))
		+ (f * ((i * o) - (k * m)))
		- (g * ((i * n) - (j * m)));

	result[1][0] =
		- (b * ((k * p) - (l * o)))
		+ (c * ((j * p) - (l * n)))
		- (d * ((j * o) - (k * n)));
	result[1][1] =
		+ (a * ((k * p) - (l * o)))
		- (c * ((i * p) - (l * m)))
		+ (d * ((i * o) - (k * m)));
	result[1][2] =
		- (a * ((j * p) - (l * n)))
		+ (b * ((i * p) - (l * m)))
		- (d * ((i * n) - (j * m)));
	result[1][3] =
		- (a * ((j * o) - (k * n)))
		+ (b * ((i * o) - (k * m)))
		- (c * ((i * n) - (j * m)));

	result[2][0] =
		+ (b * ((g * p) - (h * o)))
		- (c * ((f * p) - (h * n)))
		+ (d * ((f * o) - (g * n)));
	result[2][1] =
		- (a * ((g * p) - (h * o)))
		+ (c * ((e * p) - (h * m)))
		- (d * ((e * o) - (g * m)));
	result[2][2] =
		+ (a * ((f * p) - (h * n)))
		- (b * ((e * p) - (h * m)))
		+ (d * ((e * n) - (f * m)));
	result[2][3] =
		- (a * ((f * o) - (g * n)))
		+ (b * ((e * o) - (g * m)))
		- (c * ((e * n) - (f * m)));

	result[3][0] =
		- (b * ((g * l) - (h * k)))
		+ (c * ((f * l) - (h * j)))
		- (d * ((f * k) - (g * j)));
	result[3][1] =
		+ (a * ((g * l) - (h * k)))
		- (c * ((e * l) - (h * i)))
		+ (d * ((e * k) - (g * i)));
	result[3][2] =
		- (a * ((f * l) - (h * j)))
		+ (b * ((e * l) - (h * i)))
		- (d * ((e * j) - (f * i)));
	result[3][3] =
		- (a * ((f * k) - (g * j)))
		+ (b * ((e * k) - (g * i)))
		- (c * ((e * j) - (f * i)));

	return result;
}


Mat44 adjugate(const Mat44 &mat)
{
	return transpose(cofactor(mat));
}


int inverse(const Mat44 &inMat, Mat44 &outMat)
{
	// NOTE: (sonictk) This code is from the MESA implementation of the GLU library.
	// It (probably) expands from ``A^-1 = 1 / determinant(A) * adjugate(A)``.
	Mat44 invMat;
	invMat[0][0] =
		+ inMat[1][1] * inMat[2][2] * inMat[3][3]
		- inMat[1][1] * inMat[2][3] * inMat[3][2]
		- inMat[2][1] * inMat[1][2] * inMat[3][3]
		+ inMat[2][1] * inMat[1][3] * inMat[3][2]
		+ inMat[3][1] * inMat[1][2] * inMat[2][3]
		- inMat[3][1] * inMat[1][3] * inMat[2][2];

	invMat[1][0] =
		- inMat[1][0] * inMat[2][2] * inMat[3][3]
		+ inMat[1][0] * inMat[2][3] * inMat[3][2]
		+ inMat[2][0] * inMat[1][2] * inMat[3][3]
		- inMat[2][0] * inMat[1][3] * inMat[3][2]
		- inMat[3][0] * inMat[1][2] * inMat[2][3]
		+ inMat[3][0] * inMat[1][3] * inMat[2][2];

	invMat[2][0] =
		+ inMat[1][0] * inMat[2][1] * inMat[3][3]
		- inMat[1][0] * inMat[2][3] * inMat[3][1]
		- inMat[2][0] * inMat[1][1] * inMat[3][3]
		+ inMat[2][0] * inMat[1][3] * inMat[3][1]
		+ inMat[3][0] * inMat[1][1] * inMat[2][3]
		- inMat[3][0] * inMat[1][3] * inMat[2][1];

	invMat[3][0] =
		- inMat[1][0] * inMat[2][1] * inMat[3][2]
		+ inMat[1][0] * inMat[2][2] * inMat[3][1]
		+ inMat[2][0] * inMat[1][1] * inMat[3][2]
		- inMat[2][0] * inMat[1][2] * inMat[3][1]
		- inMat[3][0] * inMat[1][1] * inMat[2][2]
		+ inMat[3][0] * inMat[1][2] * inMat[2][1];

	invMat[0][1] =
		- inMat[0][1] * inMat[2][2] * inMat[3][3]
		+ inMat[0][1] * inMat[2][3] * inMat[3][2]
		+ inMat[2][1] * inMat[0][2] * inMat[3][3]
		- inMat[2][1] * inMat[0][3] * inMat[3][2]
		- inMat[3][1] * inMat[0][2] * inMat[2][3]
		+ inMat[3][1] * inMat[0][3] * inMat[2][2];

	invMat[1][1] =
		+ inMat[0][0] * inMat[2][2] * inMat[3][3]
		- inMat[0][0] * inMat[2][3] * inMat[3][2]
		- inMat[2][0] * inMat[0][2] * inMat[3][3]
		+ inMat[2][0] * inMat[0][3] * inMat[3][2]
		+ inMat[3][0] * inMat[0][2] * inMat[2][3]
		- inMat[3][0] * inMat[0][3] * inMat[2][2];

	invMat[2][1] =
		- inMat[0][0] * inMat[2][1] * inMat[3][3]
		+ inMat[0][0] * inMat[2][3] * inMat[3][1]
		+ inMat[2][0] * inMat[0][1] * inMat[3][3]
		- inMat[2][0] * inMat[0][3] * inMat[3][1]
		- inMat[3][0] * inMat[0][1] * inMat[2][3]
		+ inMat[3][0] * inMat[0][3] * inMat[2][1];

	invMat[3][1] =
		+ inMat[0][0] * inMat[2][1] * inMat[3][2]
		- inMat[0][0] * inMat[2][2] * inMat[3][1]
		- inMat[2][0] * inMat[0][1] * inMat[3][2]
		+ inMat[2][0] * inMat[0][2] * inMat[3][1]
		+ inMat[3][0] * inMat[0][1] * inMat[2][2]
		- inMat[3][0] * inMat[0][2] * inMat[2][1];

	invMat[0][2] =
		+ inMat[0][1] * inMat[1][2] * inMat[3][3]
		- inMat[0][1] * inMat[1][3] * inMat[3][2]
		- inMat[1][1] * inMat[0][2] * inMat[3][3]
		+ inMat[1][1] * inMat[0][3] * inMat[3][2]
		+ inMat[3][1] * inMat[0][2] * inMat[1][3]
		- inMat[3][1] * inMat[0][3] * inMat[1][2];

	invMat[1][2] =
		- inMat[0][0] * inMat[1][2] * inMat[3][3]
		+ inMat[0][0] * inMat[1][3] * inMat[3][2]
		+ inMat[1][0] * inMat[0][2] * inMat[3][3]
		- inMat[1][0] * inMat[0][3] * inMat[3][2]
		- inMat[3][0] * inMat[0][2] * inMat[1][3]
		+ inMat[3][0] * inMat[0][3] * inMat[1][2];

	invMat[2][2] =
		+ inMat[0][0] * inMat[1][1] * inMat[3][3]
		- inMat[0][0] * inMat[1][3] * inMat[3][1]
		- inMat[1][0] * inMat[0][1] * inMat[3][3]
		+ inMat[1][0] * inMat[0][3] * inMat[3][1]
		+ inMat[3][0] * inMat[0][1] * inMat[1][3]
		- inMat[3][0] * inMat[0][3] * inMat[1][1];

	invMat[3][2] =
		- inMat[0][0] * inMat[1][1] * inMat[3][2]
		+ inMat[0][0] * inMat[1][2] * inMat[3][1]
		+ inMat[1][0] * inMat[0][1] * inMat[3][2]
		- inMat[1][0] * inMat[0][2] * inMat[3][1]
		- inMat[3][0] * inMat[0][1] * inMat[1][2]
		+ inMat[3][0] * inMat[0][2] * inMat[1][1];

	invMat[0][3] =
		- inMat[0][1] * inMat[1][2] * inMat[2][3]
		+ inMat[0][1] * inMat[1][3] * inMat[2][2]
		+ inMat[1][1] * inMat[0][2] * inMat[2][3]
		- inMat[1][1] * inMat[0][3] * inMat[2][2]
		- inMat[2][1] * inMat[0][2] * inMat[1][3]
		+ inMat[2][1] * inMat[0][3] * inMat[1][2];

	invMat[1][3] =
		+ inMat[0][0] * inMat[1][2] * inMat[2][3]
		- inMat[0][0] * inMat[1][3] * inMat[2][2]
		- inMat[1][0] * inMat[0][2] * inMat[2][3]
		+ inMat[1][0] * inMat[0][3] * inMat[2][2]
		+ inMat[2][0] * inMat[0][2] * inMat[1][3]
		- inMat[2][0] * inMat[0][3] * inMat[1][2];

	invMat[2][3] =
		- inMat[0][0] * inMat[1][1] * inMat[2][3]
		+ inMat[0][0] * inMat[1][3] * inMat[2][1]
		+ inMat[1][0] * inMat[0][1] * inMat[2][3]
		- inMat[1][0] * inMat[0][3] * inMat[2][1]
		- inMat[2][0] * inMat[0][1] * inMat[1][3]
		+ inMat[2][0] * inMat[0][3] * inMat[1][1];

	invMat[3][3] =
		+ inMat[0][0] * inMat[1][1] * inMat[2][2]
		- inMat[0][0] * inMat[1][2] * inMat[2][1]
		- inMat[1][0] * inMat[0][1] * inMat[2][2]
		+ inMat[1][0] * inMat[0][2] * inMat[2][1]
		+ inMat[2][0] * inMat[0][1] * inMat[1][2]
		- inMat[2][0] * inMat[0][2] * inMat[1][1];

	float det =
		+ inMat[0][0] * outMat[0][0]
		+ inMat[0][1] * outMat[1][0]
		+ inMat[0][2]* outMat[2][0]
		+ inMat[0][3] * outMat[3][0];

	// NOTE: (sonictk) If the determinant is 0, there is no inverse matrix
	if (areFloatsEqual(det, 0.0f)) {
		return -1;
	}

	float invDet = 1.0f / det;
	for (int r=0; r <= 3; ++r) {
		for (int c=0; c <= 3; ++c) {
			outMat[r][c] = invMat[r][c] * invDet;
		}
	}

	return 0;
}
