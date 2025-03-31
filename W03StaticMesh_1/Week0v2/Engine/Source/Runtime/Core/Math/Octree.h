#pragma once

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
    void GiveOctreePadding(float padding);

    void Insert(UObject* obj);
    std::vector<UObject*>RayCast(const FVector& rayOrigin, const FVector& rayDirection);

    int objectCount = 0;
    void UpdateObjDepthBoundingBox(int inDepth);
};