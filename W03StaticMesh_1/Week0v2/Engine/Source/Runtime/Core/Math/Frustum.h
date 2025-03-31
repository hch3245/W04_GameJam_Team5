#pragma once
#include "Define.h"

struct FPlane
{
    // 평면의 방정식 계수 (ax + by + cz + d = 0)
    float a, b, c, d;

    // 점이 평면의 앞쪽(양수 방향)에 있는지 확인
    bool IsInFront(const FVector& point) const;
};

class FFrustum
{
public:
    // 6개의 절두체 평면 (근평면, 원평면, 좌평면, 우평면, 상평면, 하평면)
    FPlane nearPlane;
    FPlane farPlane;
    FPlane leftPlane;
    FPlane rightPlane;
    FPlane topPlane;
    FPlane bottomPlane;

    // 카메라 정보로부터 절두체 생성
    void ExtractFromViewProjection(const FMatrix& viewProjectionMatrix);

    // 바운딩 박스가 절두체와 교차하는지 확인
    bool IntersectsBox(const FBoundingBox& box) const;

    // 점이 절두체 내부에 있는지 확인
    bool ContainsPoint(const FVector& point) const;

private:
    // 평면 정규화
    void NormalizePlane(FPlane& plane);

    // 바운딩 박스의 모든 꼭지점이 평면 뒤에 있는지 확인
    bool CheckPlane(const FPlane& plane, const FVector corners[8]) const;
};
