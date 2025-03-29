#pragma once

#include <xmmintrin.h>

struct FVector4 {
    union {
        struct {
            float x, y, z, a;
        };
        __m128 simd;
    };

    // Default constructor
    FVector4(float _x = 0, float _y = 0, float _z = 0, float _a = 0)
        : simd(_mm_setr_ps(_x, _y, _z, _a)) {
    }

    // Constructor from __m128
    explicit FVector4(__m128 vec) : simd(vec) {}

    // Addition operator
    FVector4 operator+(const FVector4& other) const {
        return FVector4(_mm_add_ps(simd, other.simd));
    }

    // Subtraction operator
    FVector4 operator-(const FVector4& other) const {
        return FVector4(_mm_sub_ps(simd, other.simd));
    }

    // Scalar division operator
    FVector4 operator/(float scalar) const {
        __m128 scalarVec = _mm_set1_ps(scalar); // Broadcast scalar to all lanes
        return FVector4(_mm_div_ps(simd, scalarVec));
    }
};