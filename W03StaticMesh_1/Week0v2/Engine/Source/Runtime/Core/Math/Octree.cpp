#include "Octree.h"
#include "OctreeNode.h"
#include "Runtime/Launch/Define.h"
#include "Runtime/CoreUObject/UObject/Object.h"
#include "Classes/Engine/StaticMeshActor.h"

Octree::Octree(const FBoundingBox& bounds)
{
    root = new OctreeNode(bounds, 0, this);
    OctreeArray[0].Add(root);
}

Octree::Octree()
{

}

Octree::~Octree()
{
    delete root;
}

void Octree::UpdateOctreeBound(const FVector& boundMin, const FVector& boundMax)
{
    root->bounds.min = boundMin;
    root->bounds.max = boundMax;
}

void Octree::GiveOctreePadding(float padding)
{
    root->bounds.min.x -= padding;
    root->bounds.min.y -= padding;
    root->bounds.min.z -= padding;

    root->bounds.max.x += padding;
    root->bounds.max.y += padding;
    root->bounds.max.z += padding;
}

void Octree::Insert(AStaticMeshActor* obj)
{
    if (root->Contains(obj->boundingBox))
    {
        root->Insert(obj);
    }
    else
    {
        root->Insert(obj); // 루트 영역 밖이라도 일단 삽입 (필요시 트리 확장 로직 추가)
    }
}

std::vector<AStaticMeshActor*> Octree::RayCast(const FVector& rayOrigin, const FVector& rayDirection)
{
    std::vector<AStaticMeshActor*> results;
    root->RayCast(rayOrigin, rayDirection, results);
    return results;
}

std::vector<AStaticMeshActor*> Octree::FrustumCull(const FFrustum& frustum)
{
    std::vector<AStaticMeshActor*> visibleObjects;
    root->FrustumCull(frustum, visibleObjects);
    return visibleObjects;
}

void Octree::UpdateObjDepthBoundingBox(int inDepth)
{
    root->UpdateObjDepthBoundingBox(inDepth);
}

void Octree::GenerateBatches(int depth)
{
    generatedDepth = depth;
    for (auto& node : OctreeArray[generatedDepth])
    {
        node->GenerateBatches();
    }
}

void Octree::GenerateBatches()
{
    generatedDepth = maxDepth - 2;
    if (generatedDepth < 0) generatedDepth = 0;
    for (auto& node : OctreeArray[generatedDepth])
    {
        node->GenerateBatches();
    }
}

