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

// 행렬식 계산 (라플라스 전개, 4x4 행렬)
float FMatrix::Determinant(const FMatrix& Mat) {
    float det = 0.0f;
    for (int32 i = 0; i < 4; i++) {
        float subMat[3][3];
        for (int32 j = 1; j < 4; j++) {
            int32 colIndex = 0;
            for (int32 k = 0; k < 4; k++) {
                if (k == i) continue;
                subMat[j - 1][colIndex] = Mat.M[j][k];
                colIndex++;
            }
        }
        float minorDet =
            subMat[0][0] * (subMat[1][1] * subMat[2][2] - subMat[1][2] * subMat[2][1]) -
            subMat[0][1] * (subMat[1][0] * subMat[2][2] - subMat[1][2] * subMat[2][0]) +
            subMat[0][2] * (subMat[1][0] * subMat[2][1] - subMat[1][1] * subMat[2][0]);
        det += (i % 2 == 0 ? 1 : -1) * Mat.M[0][i] * minorDet;
    }
    return det;
}

// 역행렬 (가우스-조던 소거법)
FMatrix FMatrix::Inverse(const FMatrix& Mat) {
    float det = Determinant(Mat);
    if (fabs(det) < 1e-6) {
        return Identity;
    }

    FMatrix Inv;
    float invDet = 1.0f / det;

    // 여인수 행렬 계산 후 전치하여 역행렬 계산
    for (int32 i = 0; i < 4; i++) {
        for (int32 j = 0; j < 4; j++) {
            float subMat[3][3];
            int32 subRow = 0;
            for (int32 r = 0; r < 4; r++) {
                if (r == i) continue;
                int32 subCol = 0;
                for (int32 c = 0; c < 4; c++) {
                    if (c == j) continue;
                    subMat[subRow][subCol] = Mat.M[r][c];
                    subCol++;
                }
                subRow++;
            }
            float minorDet =
                subMat[0][0] * (subMat[1][1] * subMat[2][2] - subMat[1][2] * subMat[2][1]) -
                subMat[0][1] * (subMat[1][0] * subMat[2][2] - subMat[1][2] * subMat[2][0]) +
                subMat[0][2] * (subMat[1][0] * subMat[2][1] - subMat[1][1] * subMat[2][0]);

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
    __m128 vec = _mm_setr_ps(v.x, v.y, v.z, 0.0f);

    __m128 x = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0)), m.r[0]);
    __m128 y = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1)), m.r[1]);
    __m128 z = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2)), m.r[2]);

    __m128 result = _mm_add_ps(_mm_add_ps(x, y), z);

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