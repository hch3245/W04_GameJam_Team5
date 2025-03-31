#pragma once

#include "Runtime/Core/Math/Frustum.h"
#include <vector>

class OctreeNode;
struct FBoundingBox;
struct FVector;
class AStaticMeshActor;

class Octree 
{
public:
    OctreeNode* root;

    Octree(const FBoundingBox& bounds);
    Octree();
    ~Octree();

    int maxDepth = 0;

    void UpdateOctreeBound(const FVector& boundMin, const FVector& boundMax);
    void GiveOctreePadding(float padding);

    void Insert(AStaticMeshActor* obj);
    std::vector<AStaticMeshActor*>RayCast(const FVector& rayOrigin, const FVector& rayDirection);
    std::vector<AStaticMeshActor*>FrustumCull(const FFrustum& frustum);

    TArray<OctreeNode*> OctreeArray[10];

    TArray<OctreeNode*> GetBatchOctreeArray() { return OctreeArray[generatedDepth]; }

    void UpdateObjDepthBoundingBox(int inDepth);

    void GenerateBatches(int depth);

    void GenerateBatches();

    int generatedDepth = -1;
};