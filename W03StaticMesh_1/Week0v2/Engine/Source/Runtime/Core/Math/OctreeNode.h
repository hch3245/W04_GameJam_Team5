#pragma once
#include "Runtime/Launch/Define.h"
#include "Runtime/Core/Math/Frustum.h"
#include <vector>

class UObject;
struct FVector;
class Octree;

class OctreeNode
{
public:
    FBoundingBox bounds;
    std::vector<UObject*> objects;
    OctreeNode* children[8];
    bool isLeaf;
    int depth;

    Octree* octree;

    static const int MAX_OBJECTS = 5;
    static const int MAX_DEPTH = 10;

    OctreeNode(const FBoundingBox& inBounds, int inDepth = 0, Octree* inOctree = nullptr);
    ~OctreeNode();

    bool Contains(const FBoundingBox& box);
    bool Intersects(const FBoundingBox& box);
    void Subdivide();
    void Insert(UObject* obj);
    void RayCast(const FVector& rayOrigin, const FVector& rayDirection, std::vector<UObject*>& results);
    void FrustumCull(const FFrustum& frustum, std::vector<UObject*>& visibleObjects);

    void UpdateObjDepthBoundingBox(int inDepth);
};