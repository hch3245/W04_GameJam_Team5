#include "Frustum.h"

bool FPlane::IsInFront(const FVector& point) const
{
    return (a * point.x + b * point.y + c * point.z + d) > 0.0f;
}

void FFrustum::ExtractFromViewProjection(const FMatrix& viewProjectionMatrix)
{
    // 행렬의 각 행과 열에서 평면 추출
    // 좌평면 (행렬의 4열 + 1열)
    leftPlane.a = viewProjectionMatrix.M[0][3] + viewProjectionMatrix.M[0][0];
    leftPlane.b = viewProjectionMatrix.M[1][3] + viewProjectionMatrix.M[1][0];
    leftPlane.c = viewProjectionMatrix.M[2][3] + viewProjectionMatrix.M[2][0];
    leftPlane.d = viewProjectionMatrix.M[3][3] + viewProjectionMatrix.M[3][0];
    NormalizePlane(leftPlane);

    // 우평면 (행렬의 4열 - 1열)
    rightPlane.a = viewProjectionMatrix.M[0][3] - viewProjectionMatrix.M[0][0];
    rightPlane.b = viewProjectionMatrix.M[1][3] - viewProjectionMatrix.M[1][0];
    rightPlane.c = viewProjectionMatrix.M[2][3] - viewProjectionMatrix.M[2][0];
    rightPlane.d = viewProjectionMatrix.M[3][3] - viewProjectionMatrix.M[3][0];
    NormalizePlane(rightPlane);

    // 하평면 (행렬의 4열 + 2열)
    bottomPlane.a = viewProjectionMatrix.M[0][3] + viewProjectionMatrix.M[0][1];
    bottomPlane.b = viewProjectionMatrix.M[1][3] + viewProjectionMatrix.M[1][1];
    bottomPlane.c = viewProjectionMatrix.M[2][3] + viewProjectionMatrix.M[2][1];
    bottomPlane.d = viewProjectionMatrix.M[3][3] + viewProjectionMatrix.M[3][1];
    NormalizePlane(bottomPlane);

    // 상평면 (행렬의 4열 - 2열)
    topPlane.a = viewProjectionMatrix.M[0][3] - viewProjectionMatrix.M[0][1];
    topPlane.b = viewProjectionMatrix.M[1][3] - viewProjectionMatrix.M[1][1];
    topPlane.c = viewProjectionMatrix.M[2][3] - viewProjectionMatrix.M[2][1];
    topPlane.d = viewProjectionMatrix.M[3][3] - viewProjectionMatrix.M[3][1];
    NormalizePlane(topPlane);

    // 근평면 (행렬의 4열 + 3열)
    nearPlane.a = viewProjectionMatrix.M[0][3] + viewProjectionMatrix.M[0][2];
    nearPlane.b = viewProjectionMatrix.M[1][3] + viewProjectionMatrix.M[1][2];
    nearPlane.c = viewProjectionMatrix.M[2][3] + viewProjectionMatrix.M[2][2];
    nearPlane.d = viewProjectionMatrix.M[3][3] + viewProjectionMatrix.M[3][2];
    NormalizePlane(nearPlane);

    // 원평면 (행렬의 4열 - 3열)
    farPlane.a = viewProjectionMatrix.M[0][3] - viewProjectionMatrix.M[0][2];
    farPlane.b = viewProjectionMatrix.M[1][3] - viewProjectionMatrix.M[1][2];
    farPlane.c = viewProjectionMatrix.M[2][3] - viewProjectionMatrix.M[2][2];
    farPlane.d = viewProjectionMatrix.M[3][3] - viewProjectionMatrix.M[3][2];
    NormalizePlane(farPlane);
}

bool FFrustum::IntersectsBox(const FBoundingBox& box) const
{
    // 바운딩 박스의 8개 꼭지점
    FVector corners[8];
    corners[0] = FVector(box.min.x, box.min.y, box.min.z);
    corners[1] = FVector(box.max.x, box.min.y, box.min.z);
    corners[2] = FVector(box.min.x, box.max.y, box.min.z);
    corners[3] = FVector(box.max.x, box.max.y, box.min.z);
    corners[4] = FVector(box.min.x, box.min.y, box.max.z);
    corners[5] = FVector(box.max.x, box.min.y, box.max.z);
    corners[6] = FVector(box.min.x, box.max.y, box.max.z);
    corners[7] = FVector(box.max.x, box.max.y, box.max.z);

    // 각 평면에 대해 검사
    if (!CheckPlane(nearPlane, corners)) return false;
    if (!CheckPlane(farPlane, corners)) return false;
    if (!CheckPlane(leftPlane, corners)) return false;
    if (!CheckPlane(rightPlane, corners)) return false;
    if (!CheckPlane(topPlane, corners)) return false;
    if (!CheckPlane(bottomPlane, corners)) return false;

    return true;
}

bool FFrustum::ContainsPoint(const FVector& point) const
{
    if (!nearPlane.IsInFront(point)) return false;
    if (!farPlane.IsInFront(point)) return false;
    if (!leftPlane.IsInFront(point)) return false;
    if (!rightPlane.IsInFront(point)) return false;
    if (!topPlane.IsInFront(point)) return false;
    if (!bottomPlane.IsInFront(point)) return false;

    return true;
}

void FFrustum::NormalizePlane(FPlane& plane)
{
    float magnitude = sqrtf(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
    if (magnitude > 0.0f)
    {
        plane.a /= magnitude;
        plane.b /= magnitude;
        plane.c /= magnitude;
        plane.d /= magnitude;
    }
}

bool FFrustum::CheckPlane(const FPlane& plane, const FVector corners[8]) const
{
    for (int i = 0; i < 8; i++)
    {
        if (plane.IsInFront(corners[i]))
            return true;
    }
    return false;
}
