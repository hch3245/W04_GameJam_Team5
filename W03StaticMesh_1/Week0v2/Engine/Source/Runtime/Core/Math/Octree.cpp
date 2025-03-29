#include "Octree.h"
#include "OctreeNode.h"
#include "Runtime/Launch/Define.h"
#include "Runtime/CoreUObject/UObject/Object.h"

Octree::Octree(const FBoundingBox& bounds)
{
    root = new OctreeNode(bounds, 0, this);
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

void Octree::Insert(UObject* obj)
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

std::vector<UObject*> Octree::RayCast(const FVector& rayOrigin, const FVector& rayDirection)
{
    std::vector<UObject*> results;
    root->RayCast(rayOrigin, rayDirection, results);
    return results;
}
