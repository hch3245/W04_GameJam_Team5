#pragma once

#include <xmmintrin.h>
#include <pmmintrin.h>

struct FMatrix
{
    union {
        struct {
            float M[4][4];
        };
        __m128 r[4];
    };

    FMatrix() {
        for (int i = 0; i < 4; ++i) {
            r[i] = _mm_setzero_ps();
        }
    }

    FMatrix(const float mat[4][4]) {
        for (int i = 0; i < 4; ++i) {
            r[i] = _mm_loadu_ps(mat[i]);
        }
    }

    FMatrix(__m128 row0, __m128 row1, __m128 row2, __m128 row3) {
        r[0] = row0;
        r[1] = row1;
        r[2] = row2;
        r[3] = row3;
    }

	static const FMatrix Identity;
	// 기본 연산자 오버로딩
	FMatrix operator+(const FMatrix& Other) const;
	FMatrix operator-(const FMatrix& Other) const;
	FMatrix operator*(const FMatrix& Other) const;
	FMatrix operator*(float Scalar) const;
	FMatrix operator/(float Scalar) const;
	float* operator[](int row);
	const float* operator[](int row) const;
	
	// 유틸리티 함수
	static FMatrix Transpose(const FMatrix& Mat);
	static float Determinant(const FMatrix& Mat);
	static FMatrix Inverse(const FMatrix& Mat);
    static FMatrix InverseByXMMatrix(const FMatrix& Mat);
	static FMatrix CreateRotation(float roll, float pitch, float yaw);
	static FMatrix CreateScale(float scaleX, float scaleY, float scaleZ);
	static FVector TransformVector(const FVector& v, const FMatrix& m);
	static FVector4 TransformVector(const FVector4& v, const FMatrix& m);
	static FMatrix CreateTranslationMatrix(const FVector& position);

    FVector TransformPosition(const FVector& vector) const {
        // Set up the vector with w = 1.0f in a register.
        __m128 vec = _mm_setr_ps(vector.x, vector.y, vector.z, 1.0f);

        // Since r is stored in column-major order, treat r[0]..r[3] as columns.
        __m128 col0 = r[0]; // Contains: m00, m10, m20, m30
        __m128 col1 = r[1]; // Contains: m01, m11, m21, m31
        __m128 col2 = r[2]; // Contains: m02, m12, m22, m32
        __m128 col3 = r[3]; // Contains: m03, m13, m23, m33

        // Multiply each column by the corresponding component of the vector.
        __m128 mul0 = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0)), col0);
        __m128 mul1 = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1)), col1);
        __m128 mul2 = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2)), col2);
        __m128 mul3 = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3)), col3);

        // Sum the results: result = vector.x * col0 + vector.y * col1 + vector.z * col2 + 1.0f * col3
        __m128 result = _mm_add_ps(_mm_add_ps(mul0, mul1), _mm_add_ps(mul2, mul3));

        // Store the result to a temporary array.
        alignas(16) float arr[4];
        _mm_store_ps(arr, result);

        // Use an epsilon threshold to check for a near-zero w value.
        constexpr float epsilon = 1e-6f;
        if (fabsf(arr[3]) > epsilon)
            return FVector(arr[0] / arr[3], arr[1] / arr[3], arr[2] / arr[3]);
        else
            return FVector(arr[0], arr[1], arr[2]);
    }
};