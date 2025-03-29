#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include <DirectXMath.h>

#include "MathUtility.h"

using namespace DirectX;

FVector4 JungleMath::ConvertV3ToV4(FVector vec3)
{
    return FVector4{ vec3.x, vec3.y, vec3.z };
}

FMatrix JungleMath::CreateModelMatrix(FVector translation, FVector rotation, FVector scale)
{
    FMatrix Translation = FMatrix::CreateTranslationMatrix(translation);

    FMatrix Rotation = FMatrix::CreateRotation(rotation.x, rotation.y, rotation.z);

    FMatrix Scale = FMatrix::CreateScale(scale.x, scale.y, scale.z);
    return Scale * Rotation * Translation;
}

FMatrix JungleMath::CreateModelMatrix(FVector translation, FQuat rotation, FVector scale)
{
    FMatrix Translation = FMatrix::CreateTranslationMatrix(translation);
    FMatrix Rotation = rotation.ToMatrix();
    FMatrix Scale = FMatrix::CreateScale(scale.x, scale.y, scale.z);
    return Scale * Rotation * Translation;
}

FMatrix JungleMath::CreateViewMatrix(FVector eye, FVector target, FVector up)
{
    FVector zAxis = (target - eye).Normalize();  // DirectX는 LH이므로 -z가 아니라 +z 사용
    FVector xAxis = (up.Cross(zAxis)).Normalize();
    FVector yAxis = zAxis.Cross(xAxis);

    return FMatrix{
        _mm_setr_ps(xAxis.x, yAxis.x, zAxis.x,0.0f),
        _mm_setr_ps(xAxis.y, yAxis.y, zAxis.y,0.0f),
        _mm_setr_ps(xAxis.z, yAxis.z, zAxis.z,0.0f),
        _mm_setr_ps(-xAxis.Dot(eye), -yAxis.Dot(eye), -zAxis.Dot(eye), 1.0f),
    };
}

FMatrix JungleMath::CreateProjectionMatrix(float fov, float aspect, float nearPlane, float farPlane) {
    float tanHalfFOV = tanf(fov / 2.0f);
    float depth = farPlane - nearPlane;

    // Precompute values for the projection matrix
    float m00 = 1.0f / (aspect * tanHalfFOV);
    float m11 = 1.0f / tanHalfFOV;
    float m22 = farPlane / depth;
    float m32 = -(nearPlane * farPlane) / depth;

    // Construct the projection matrix using SIMD
    return FMatrix{
        _mm_setr_ps(m00, 0.0f, 0.0f, 0.0f),
        _mm_setr_ps(0.0f, m11, 0.0f, 0.0f),
        _mm_setr_ps(0.0f, 0.0f, m22, 1.0f),
        _mm_setr_ps(0.0f, 0.0f, m32, 0.0f) 
    };
}


FMatrix JungleMath::CreateOrthoProjectionMatrix(float width, float height, float nearPlane, float farPlane)
{
    float r = width * 0.5f;
    float t = height * 0.5f;
    float invDepth = 1.0f / (farPlane - nearPlane);

    FMatrix Projection = {};
    Projection.M[0][0] = 1.0f / r;
    Projection.M[1][1] = 1.0f / t;
    Projection.M[2][2] = invDepth;
    Projection.M[3][2] = -nearPlane * invDepth;
    Projection.M[3][3] = 1.0f;

    return Projection;
}

FVector JungleMath::FVectorRotate(FVector& origin, const FVector& rotation)
{
    FQuat quaternion = JungleMath::EulerToQuaternion(rotation);
    // 쿼터니언을 이용해 벡터 회전 적용
    return quaternion.RotateVector(origin);
}
FQuat JungleMath::EulerToQuaternion(const FVector& eulerDegrees)
{
    float yaw = DegToRad(eulerDegrees.z);   
    float pitch = DegToRad(eulerDegrees.y); 
    float roll = DegToRad(eulerDegrees.x);  

    float halfYaw = yaw * 0.5f;
    float halfPitch = pitch * 0.5f;
    float halfRoll = roll * 0.5f;

    float cosYaw = cos(halfYaw);
    float sinYaw = sin(halfYaw);
    float cosPitch = cos(halfPitch);
    float sinPitch = sin(halfPitch);
    float cosRoll = cos(halfRoll);
    float sinRoll = sin(halfRoll);

    FQuat quat;
    quat.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
    quat.x = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
    quat.y = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
    quat.z = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;

    quat.Normalize();
    return quat;
}

FVector JungleMath::QuaternionToEuler(const FQuat& quat)
{
    FVector euler;

    // 쿼터니언 정규화
    FQuat q = quat;
    q.Normalize();

    // Yaw (Z 축 회전)
    float sinYaw = 2.0f * (q.w * q.z + q.x * q.y);
    float cosYaw = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    euler.z = RadToDeg(atan2(sinYaw, cosYaw));

    // Pitch (Y 축 회전, 짐벌락 방지)
    float sinPitch = 2.0f * (q.w * q.y - q.z * q.x);
    if (fabs(sinPitch) >= 1.0f)
    {
        euler.y = RadToDeg(static_cast<float>(copysign(PI / 2, sinPitch))); // Gimbal Lock 방지
    }
    else
    {
        euler.y = RadToDeg(asin(sinPitch));
    }

    // Roll (X 축 회전)
    float sinRoll = 2.0f * (q.w * q.x + q.y * q.z);
    float cosRoll = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    euler.x = RadToDeg(atan2(sinRoll, cosRoll));
    return euler;
}

FVector JungleMath::FVectorRotate(FVector& origin, const FQuat& rotation)
{
    return rotation.RotateVector(origin);
}

FMatrix JungleMath::CreateRotationMatrix(FVector rotation)
{
    XMVECTOR quatX = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), DegToRad(rotation.x));
    XMVECTOR quatY = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), DegToRad(rotation.y));
    XMVECTOR quatZ = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), DegToRad(rotation.z));

    XMVECTOR rotationQuat = XMQuaternionMultiply(quatZ, XMQuaternionMultiply(quatY, quatX));
    rotationQuat = XMQuaternionNormalize(rotationQuat);  // 정규화 필수

    XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotationQuat);
    FMatrix result = FMatrix::Identity;  // 기본값 설정 (단위 행렬)

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.M[i][j] = rotationMatrix.r[i].m128_f32[j];  // XMMATRIX에서 FMatrix로 값 복사
        }
    }
    return result;
}

float JungleMath::RadToDeg(float radian)
{
    return static_cast<float>(radian * (180.0f / PI));
}

float JungleMath::DegToRad(float degree)
{
    return static_cast<float>(degree * (PI / 180.0f));
}
