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

	FVector4 TransformFVector4(const FVector4& vector)
	{
		return FVector4(
			M[0][0] * vector.x + M[1][0] * vector.y + M[2][0] * vector.z + M[3][0] * vector.a,
			M[0][1] * vector.x + M[1][1] * vector.y + M[2][1] * vector.z + M[3][1] * vector.a,
			M[0][2] * vector.x + M[1][2] * vector.y + M[2][2] * vector.z + M[3][2] * vector.a,
			M[0][3] * vector.x + M[1][3] * vector.y + M[2][3] * vector.z + M[3][3] * vector.a
		);
	}
	FVector TransformPosition(const FVector& vector) const
	{
		float x = M[0][0] * vector.x + M[1][0] * vector.y + M[2][0] * vector.z + M[3][0];
		float y = M[0][1] * vector.x + M[1][1] * vector.y + M[2][1] * vector.z + M[3][1];
		float z = M[0][2] * vector.x + M[1][2] * vector.y + M[2][2] * vector.z + M[3][2];
		float w = M[0][3] * vector.x + M[1][3] * vector.y + M[2][3] * vector.z + M[3][3];
		return w != 0.0f ? FVector{ x / w, y / w, z / w } : FVector{ x, y, z };
	}
};