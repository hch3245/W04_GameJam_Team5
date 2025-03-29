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
	static FMatrix CreateRotation(float roll, float pitch, float yaw);
	static FMatrix CreateScale(float scaleX, float scaleY, float scaleZ);
	static FVector TransformVector(const FVector& v, const FMatrix& m);
	static FVector4 TransformVector(const FVector4& v, const FMatrix& m);
	static FMatrix CreateTranslationMatrix(const FVector& position);

	FVector TransformPosition(const FVector& vector) const {

        __m128 vec = _mm_setr_ps(vector.x, vector.y, vector.z, 1.0f);

        __m128 x = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0)), r[0]);
        __m128 y = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1)), r[1]);
        __m128 z = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2)), r[2]);
        __m128 w = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3)), r[3]);

        __m128 result = _mm_add_ps(_mm_add_ps(x, y), _mm_add_ps(z, w));

        alignas(16) float arr[4];
        _mm_store_ps(arr, result);

        return (arr[3] != 0.0f) ?
            FVector(arr[0] / arr[3], arr[1] / arr[3], arr[2] / arr[3]) :
            FVector(arr[0], arr[1], arr[2]);
    }
};