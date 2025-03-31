#pragma once

#include "Runtime/Core/Math/Frustum.h"
#include <vector>

class OctreeNode;
struct FBoundingBox;
struct FVector;
class UObject;

class Octree 
{
public:
    OctreeNode* root;

    Octree(const FBoundingBox& bounds);
    Octree();
    ~Octree();

    void UpdateOctreeBound(const FVector& boundMin, const FVector& boundMax);

    void Insert(UObject* obj);
    std::vector<UObject*>RayCast(const FVector& rayOrigin, const FVector& rayDirection);
    std::vector<UObject*>FrustumCull(const FFrustum& frustum);

    int objectCount = 0;
};