#include "Define.h"

// 단위 행렬 정의
const FMatrix FMatrix::Identity = {
    _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f),
    _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f),
    _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f),
    _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f) 
};

// 행렬 덧셈
FMatrix FMatrix::operator+(const FMatrix& Other) const {
    FMatrix result;
    result.r[0] = _mm_add_ps(r[0], Other.r[0]);
    result.r[1] = _mm_add_ps(r[1], Other.r[1]);
    result.r[2] = _mm_add_ps(r[2], Other.r[2]);
    result.r[3] = _mm_add_ps(r[3], Other.r[3]);
    return result;
}

// 행렬 뺄셈
FMatrix FMatrix::operator-(const FMatrix& Other) const {
    FMatrix result;
    result.r[0] = _mm_sub_ps(r[0], Other.r[0]);
    result.r[1] = _mm_sub_ps(r[1], Other.r[1]);
    result.r[2] = _mm_sub_ps(r[2], Other.r[2]);
    result.r[3] = _mm_sub_ps(r[3], Other.r[3]);
    return result;
}

// 행렬 곱셈
FMatrix FMatrix::operator*(const FMatrix& Other) const {
    FMatrix result;

    for (int i = 0; i < 4; ++i) {
        __m128 row = r[i];
        result.r[i] = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(row, row, _MM_SHUFFLE(0, 0, 0, 0)), Other.r[0]),
                _mm_mul_ps(_mm_shuffle_ps(row, row, _MM_SHUFFLE(1, 1, 1, 1)), Other.r[1])
            ),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(row, row, _MM_SHUFFLE(2, 2, 2, 2)), Other.r[2]),
                _mm_mul_ps(_mm_shuffle_ps(row, row, _MM_SHUFFLE(3, 3, 3, 3)), Other.r[3])
            )
        );
    }
    return result;
}

// 스칼라 곱셈
FMatrix FMatrix::operator*(float Scalar) const {
    FMatrix result;
    __m128 scalarVector = _mm_set_ps(Scalar, Scalar, Scalar, Scalar);

    result.r[0] = _mm_mul_ps(r[0], scalarVector);
    result.r[1] = _mm_mul_ps(r[1], scalarVector);
    result.r[2] = _mm_mul_ps(r[2], scalarVector);
    result.r[3] = _mm_mul_ps(r[3], scalarVector);
    return result;
}

FMatrix FMatrix::operator/(float Scalar) const {
    FMatrix result;
    __m128 scalarVec = _mm_set1_ps(1.0f / Scalar);

    result.r[0] = _mm_mul_ps(r[0], scalarVec);
    result.r[1] = _mm_mul_ps(r[1], scalarVec);
    result.r[2] = _mm_mul_ps(r[2], scalarVec);
    result.r[3] = _mm_mul_ps(r[3], scalarVec);

    return result;
}

float* FMatrix::operator[](int row) {
    return M[row];
}

const float* FMatrix::operator[](int row) const
{
    return M[row];
}

// 전치 행렬
FMatrix FMatrix::Transpose(const FMatrix& Mat) {
    FMatrix result = Mat;
    _MM_TRANSPOSE4_PS(result.r[0], result.r[1], result.r[2], result.r[3]);
    return result;
}

float FMatrix::Determinant(const FMatrix& Mat) {
    // 4x4 행렬의 행렬식 계산을 위한 최적화된 방법
    // 2x2 소행렬식 계산
    __m128 m0 = _mm_loadu_ps(&Mat.M[0][0]); // 첫 번째 행
    __m128 m1 = _mm_loadu_ps(&Mat.M[1][0]); // 두 번째 행
    __m128 m2 = _mm_loadu_ps(&Mat.M[2][0]); // 세 번째 행
    __m128 m3 = _mm_loadu_ps(&Mat.M[3][0]); // 네 번째 행

    // 2x2 소행렬식 계산을 위한 셔플
    __m128 s0 = _mm_shuffle_ps(m2, m2, _MM_SHUFFLE(2, 3, 1, 0));
    __m128 s1 = _mm_shuffle_ps(m3, m3, _MM_SHUFFLE(2, 3, 1, 0));
    __m128 s2 = _mm_shuffle_ps(m2, m2, _MM_SHUFFLE(1, 0, 3, 2));
    __m128 s3 = _mm_shuffle_ps(m3, m3, _MM_SHUFFLE(1, 0, 3, 2));

    // 2x2 소행렬식 계산
    __m128 d0 = _mm_mul_ps(s0, s3);
    __m128 d1 = _mm_mul_ps(s1, s2);
    __m128 d = _mm_sub_ps(d0, d1);

    // 첫 번째 행과 소행렬식 곱하기
    __m128 s4 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(2, 3, 1, 0));
    __m128 s5 = _mm_shuffle_ps(d, d, _MM_SHUFFLE(2, 3, 1, 0));
    __m128 s6 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(1, 0, 3, 2));
    __m128 s7 = _mm_shuffle_ps(d, d, _MM_SHUFFLE(1, 0, 3, 2));

    __m128 m4 = _mm_mul_ps(s4, s5);
    __m128 m5 = _mm_mul_ps(s6, s7);
    __m128 r0 = _mm_sub_ps(m4, m5);

    // 부호 조정 및 첫 번째 행과 곱하기
    __m128 sign = _mm_set_ps(-1.0f, 1.0f, -1.0f, 1.0f);
    r0 = _mm_mul_ps(r0, sign);
    r0 = _mm_mul_ps(r0, m0);

    // 수평 합계
    r0 = _mm_hadd_ps(r0, r0);
    r0 = _mm_hadd_ps(r0, r0);

    float det;
    _mm_store_ss(&det, r0);

    return det;
}

// SIMD를 활용한 역행렬 계산
FMatrix FMatrix::Inverse(const FMatrix& Mat) {
    // 행렬식 계산
    float det = Determinant(Mat);
    if (fabs(det) < 1e-6) {
        return Identity;
    }

    FMatrix Inv;
    float invDet = 1.0f / det;
    __m128 vInvDet = _mm_set1_ps(invDet);

    // 4x4 행렬을 위한 보조 행렬 준비
    __m128 row0 = _mm_loadu_ps(&Mat.M[0][0]);
    __m128 row1 = _mm_loadu_ps(&Mat.M[1][0]);
    __m128 row2 = _mm_loadu_ps(&Mat.M[2][0]);
    __m128 row3 = _mm_loadu_ps(&Mat.M[3][0]);

    // 전치 행렬 계산을 위한 준비
    __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
    __m128 tmp1 = _mm_unpacklo_ps(row2, row3);
    __m128 tmp2 = _mm_unpackhi_ps(row0, row1);
    __m128 tmp3 = _mm_unpackhi_ps(row2, row3);

    __m128 col0 = _mm_movelh_ps(tmp0, tmp1);
    __m128 col1 = _mm_movehl_ps(tmp1, tmp0);
    __m128 col2 = _mm_movelh_ps(tmp2, tmp3);
    __m128 col3 = _mm_movehl_ps(tmp3, tmp2);

    // 여인수 행렬 계산
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            // 3x3 소행렬 추출
            float subMat[3][3];
            int subRow = 0;
            for (int r = 0; r < 4; r++) {
                if (r == i) continue;
                int subCol = 0;
                for (int c = 0; c < 4; c++) {
                    if (c == j) continue;
                    subMat[subRow][subCol] = Mat.M[r][c];
                    subCol++;
                }
                subRow++;
            }

            // 3x3 소행렬식 계산 (SIMD 활용)
            __m128 r0 = _mm_set_ps(0.0f, subMat[0][2], subMat[0][1], subMat[0][0]);
            __m128 r1 = _mm_set_ps(0.0f, subMat[1][2], subMat[1][1], subMat[1][0]);
            __m128 r2 = _mm_set_ps(0.0f, subMat[2][2], subMat[2][1], subMat[2][0]);

            // 첫 번째 항: subMat[0][0] * (subMat[1][1] * subMat[2][2] - subMat[1][2] * subMat[2][1])
            __m128 m1 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0, 0, 3, 1));
            __m128 m2 = _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(0, 0, 2, 2));
            __m128 m3 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0, 0, 2, 2));
            __m128 m4 = _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(0, 0, 1, 1));

            __m128 p1 = _mm_mul_ps(m1, m2);
            __m128 p2 = _mm_mul_ps(m3, m4);
            __m128 s1 = _mm_sub_ps(p1, p2);

            // 두 번째 항: -subMat[0][1] * (subMat[1][0] * subMat[2][2] - subMat[1][2] * subMat[2][0])
            m1 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0, 0, 3, 0));
            m2 = _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(0, 0, 2, 2));
            m3 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0, 0, 2, 2));
            m4 = _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(0, 0, 0, 0));

            p1 = _mm_mul_ps(m1, m2);
            p2 = _mm_mul_ps(m3, m4);
            __m128 s2 = _mm_sub_ps(p1, p2);
            s2 = _mm_mul_ps(s2, _mm_set1_ps(-1.0f));

            // 세 번째 항: subMat[0][2] * (subMat[1][0] * subMat[2][1] - subMat[1][1] * subMat[2][0])
            m1 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0, 0, 3, 0));
            m2 = _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(0, 0, 1, 1));
            m3 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0, 0, 1, 1));
            m4 = _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(0, 0, 0, 0));

            p1 = _mm_mul_ps(m1, m2);
            p2 = _mm_mul_ps(m3, m4);
            __m128 s3 = _mm_sub_ps(p1, p2);

            // 첫 번째 행과 곱하기
            __m128 t1 = _mm_mul_ps(_mm_shuffle_ps(r0, r0, _MM_SHUFFLE(0, 0, 0, 0)), s1);
            __m128 t2 = _mm_mul_ps(_mm_shuffle_ps(r0, r0, _MM_SHUFFLE(0, 0, 1, 1)), s2);
            __m128 t3 = _mm_mul_ps(_mm_shuffle_ps(r0, r0, _MM_SHUFFLE(0, 0, 2, 2)), s3);

            // 합산
            __m128 sum = _mm_add_ps(t1, _mm_add_ps(t2, t3));

            // 수평 합계
            sum = _mm_hadd_ps(sum, sum);
            sum = _mm_hadd_ps(sum, sum);

            float minorDet;
            _mm_store_ss(&minorDet, sum);

            // 부호 조정 및 역행렬 요소 계산
            Inv.M[j][i] = ((i + j) % 2 == 0 ? 1 : -1) * minorDet * invDet;
        }
    }

    return Inv;
}



FMatrix FMatrix::CreateRotation(float roll, float pitch, float yaw) {
    // Convert degrees to radians
    constexpr float Deg2Rad = 3.14159265359f / 180.0f;
    float radRoll = roll * Deg2Rad;
    float radPitch = pitch * Deg2Rad;
    float radYaw = yaw * Deg2Rad;

    // Precompute trigonometric values
    float cosR = cosf(radRoll), sinR = sinf(radRoll);
    float cosP = cosf(radPitch), sinP = sinf(radPitch);
    float cosY = cosf(radYaw), sinY = sinf(radYaw);

    // Z-axis rotation matrix (Yaw)
    FMatrix rotationZ = {
        _mm_setr_ps(cosY,  sinY, 0.0f, 0.0f),
        _mm_setr_ps(-sinY, cosY, 0.0f, 0.0f),
        _mm_setr_ps(0.0f,  0.0f, 1.0f, 0.0f),
        _mm_setr_ps(0.0f,  0.0f, 0.0f, 1.0f)
    };

    // Y-axis rotation matrix (Pitch)
    FMatrix rotationY = {
        _mm_setr_ps(cosP, 0.0f, -sinP, 0.0f),
        _mm_setr_ps(0.0f, 1.0f,  0.0f, 0.0f),
        _mm_setr_ps(sinP, 0.0f,  cosP, 0.0f),
        _mm_setr_ps(0.0f, 0.0f,  0.0f, 1.0f)
    };

    // X-axis rotation matrix (Roll)
    FMatrix rotationX = {
        _mm_setr_ps(1.0f,  0.0f,   0.0f,   0.0f),
        _mm_setr_ps(0.0f, cosR,   sinR,   0.0f),
        _mm_setr_ps(0.0f, -sinR, cosR,   0.0f),
        _mm_setr_ps(0.0f,  0.0f,   0.0f,   1.0f)
    };

    // Combine rotations in Z(Yaw) → Y(Pitch) → X(Roll) order
    return rotationX * rotationY * rotationZ;
}



// 스케일 행렬 생성
FMatrix FMatrix::CreateScale(float scaleX, float scaleY, float scaleZ)
{
    return FMatrix{
        _mm_setr_ps(scaleX, 0.0f, 0.0f, 0.0f),
        _mm_setr_ps(0.0f, scaleY, 0.0f, 0.0f),
        _mm_setr_ps(0.0f, 0.0f, scaleZ, 0.0f),
        _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f)
    };
}

FMatrix FMatrix::CreateTranslationMatrix(const FVector& position) {
    return FMatrix{
        _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f),                   
        _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f),                   
        _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f),                  
        _mm_setr_ps(position.x, position.y, position.z, 1.0f)
    };
}

FVector FMatrix::TransformVector(const FVector& v, const FMatrix& m) {
    __m128 vec = _mm_setr_ps(v.x, v.y, v.z, 1.0f);

    __m128 x = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0)), m.r[0]);
    __m128 y = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1)), m.r[1]);
    __m128 z = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2)), m.r[2]);

    __m128 result = _mm_add_ps(_mm_add_ps(_mm_add_ps(x, y), z), m.r[3]);

    alignas(16) float arr[4];
    _mm_store_ps(arr, result);
    return FVector(arr[0], arr[1], arr[2]);
}


FVector4 FMatrix::TransformVector(const FVector4& v, const FMatrix& m) {
    __m128 vec = _mm_setr_ps(v.x, v.y, v.z, v.a);

    __m128 x = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0)), m.r[0]);
    __m128 y = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1)), m.r[1]);
    __m128 z = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2)), m.r[2]);
    __m128 w = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3)), m.r[3]);

    __m128 result = _mm_add_ps(_mm_add_ps(x, y), _mm_add_ps(z, w));

    return FVector4(result);
}