#pragma once

#include <cmath>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>

struct alignas(16) FVector2D
{
    union {
        struct {
            float x, y;
        };
        __m128 simd;
    };

    FVector2D(float _x = 0, float _y = 0)
    {
        simd = _mm_set_ps(0, 0, _y, _x);
    }

    FVector2D(__m128 _simd)
    {
        simd = _simd;
    }

    FVector2D operator+(const FVector2D& rhs) const
    {
        __m128 result = _mm_add_ps(simd, rhs.simd);
        return FVector2D(result);
    }

    FVector2D operator-(const FVector2D& rhs) const
    {
        __m128 result = _mm_sub_ps(simd, rhs.simd);
        return FVector2D(result);
    }

    FVector2D operator*(float rhs) const
    {
        __m128 scalar = _mm_set1_ps(rhs);
        __m128 result = _mm_mul_ps(simd, scalar);
        return FVector2D(result);
    }

    FVector2D operator/(float rhs) const
    {
        __m128 scalar = _mm_set1_ps(rhs);
        __m128 result = _mm_div_ps(simd, scalar);
        return FVector2D(result);
    }

    FVector2D& operator+=(const FVector2D& rhs)
    {
        simd = _mm_add_ps(simd, rhs.simd);
        return *this;
    }
};

struct alignas(16) FVector
{
    union {
        struct {
            float x, y, z;
        };
        __m128 simd;
    };

    FVector(float _x = 0, float _y = 0, float _z = 0)
    {
        simd = _mm_set_ps(0.0f, _z, _y, _x);
    }

    FVector(__m128 _simd)
    {
        simd = _simd;
    }

    FVector operator+(const FVector& other) const {
        __m128 result = _mm_add_ps(simd, other.simd);
        return FVector(result);
    }

    FVector operator-(const FVector& other) const
    {
        __m128 result = _mm_sub_ps(simd, other.simd);
        return FVector(result);
    }

    float Dot(const FVector& other) const {
        __m128 result = _mm_dp_ps(simd, other.simd, 0x7F);
        return _mm_cvtss_f32(result);
    }

    float Magnitude() const {
        __m128 result = _mm_dp_ps(simd, simd, 0x7F);
        return sqrtf(_mm_cvtss_f32(result));
    }

    FVector Normalize() const
    {
        float mag = Magnitude();
        if (mag > 0.0f) {
            __m128 scalar = _mm_set1_ps(1.0f / mag);
            __m128 result = _mm_mul_ps(simd, scalar);
            return FVector(result.m128_f32[0], result.m128_f32[1], result.m128_f32[2]);
        }
        return FVector(0.0f, 0.0f, 0.0f);
    }

    FVector Cross(const FVector& other) const
    {
        __m128 a_yzx = _mm_shuffle_ps(simd, simd, _MM_SHUFFLE(3, 0, 2, 1));   // yzx 순서로 셔플링
        __m128 b_zxy = _mm_shuffle_ps(other.simd, other.simd, _MM_SHUFFLE(3, 1, 0, 2)); // zxy 순서로 셔플링
        __m128 cross1 = _mm_mul_ps(a_yzx, b_zxy);                             // y*z', z*x', x*y'

        __m128 a_zxy = _mm_shuffle_ps(simd, simd, _MM_SHUFFLE(3, 1, 0, 2));   // zxy 순서로 셔플링
        __m128 b_yzx = _mm_shuffle_ps(other.simd, other.simd, _MM_SHUFFLE(3, 0, 2, 1)); // yzx 순서로 셔플링
        __m128 cross2 = _mm_mul_ps(a_zxy, b_yzx);                             // z*y', x*z', y*x'

        return FVector(_mm_sub_ps(cross1, cross2));
    }

    FVector operator*(float scalar) const
    {
        __m128 scalarVec = _mm_set1_ps(scalar);
        __m128 result = _mm_mul_ps(simd, scalarVec);
        return FVector(result.m128_f32[0], result.m128_f32[1], result.m128_f32[2]);
    }

    bool operator==(const FVector& other) const
    {
        return (x == other.x && y == other.y && z == other.z);
    }

    float Distance(const FVector& other) const
    {
        return ((*this - other).Magnitude());
    }

    static const FVector ZeroVector;
    static const FVector OneVector;
    static const FVector UpVector;
    static const FVector ForwardVector;
    static const FVector RightVector;
};
