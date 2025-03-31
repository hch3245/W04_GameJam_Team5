#pragma once
#include "Runtime/Launch/Define.h"
#include "Runtime/Core/Math/Frustum.h"
#include <vector>

class AStaticMeshActor;
struct FVector;
class Octree;

class OctreeNode
{
public:
    FBoundingBox bounds;
    std::vector<AStaticMeshActor*> objects;
    int holdObjNum;
    OctreeNode* children[8];
    bool isLeaf;
    int depth;

    // 원본 OBJ Batch의 Index
    int OriginalOBJBatchIndex = -1;
    // LOD1 OBJ Batch의 Index
    int LOD1OBJBatchIndex = -1;
    // LOD2 OBJ Batch의 Idnex
    int LOD2OBJBatchIndex = -1;

    const float OriginalLength = 5.0f;
    const float LOD1Length = 10.0f;
    const float LOD2Length = 20.0f;

    Octree* octree;

    static const int MAX_OBJECTS = 5;
    static const int MAX_DEPTH = 10;

    OctreeNode(const FBoundingBox& inBounds, int inDepth = 0, Octree* inOctree = nullptr);
    ~OctreeNode();

    bool Contains(const FBoundingBox& box);
    bool Intersects(const FBoundingBox& box);
    void Subdivide();
    void Insert(AStaticMeshActor* obj);
    void RayCast(const FVector& rayOrigin, const FVector& rayDirection, std::vector<AStaticMeshActor*>& results);
    void FrustumCull(const FFrustum& frustum, std::vector<AStaticMeshActor*>& visibleObjects);

    void UpdateObjDepthBoundingBox(int inDepth);

    void GenerateBatches();

    TArray<AStaticMeshActor*> GetObjectsIncludeChildren();

    int GiveOBJBatchIndex(const FVector& cameraPosition, int MaterialNum, int LODLevel);
};