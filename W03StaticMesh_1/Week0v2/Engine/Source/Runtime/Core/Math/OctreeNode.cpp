#include "OctreeNode.h"
#include "Vector.h"
#include "Runtime/CoreUObject/UObject/Object.h"
#include "Octree.h"
#include "UnrealEd/PrimitiveBatch.h"

OctreeNode::OctreeNode(const FBoundingBox& inBounds, int inDepth, Octree* inOctree)
    :bounds(inBounds), isLeaf(true), depth(inDepth), octree(inOctree)
{
    UPrimitiveBatch::GetInstance().AddOctreeAABB(bounds, depth);

    for (int i = 0; i < 8; i++)
    {
        children[i] = nullptr;
    }
}

OctreeNode::~OctreeNode()
{
    for (int i = 0; i < 8; i++)
    {
        if (children[i])
        {
            delete children[i];
            children[i] = nullptr;
        }
    }
}

bool OctreeNode::Contains(const FBoundingBox& box)
{
    return bounds.BoxIntersect(box.min, box.max);
}

void OctreeNode::Subdivide()
{
    FVector center = (bounds.min + bounds.max) * 0.5f;
    
    // 8개 영역을 아래와 같이 정의 그림으로 검증 완료
    children[0] = new OctreeNode(FBoundingBox(bounds.min, center), depth + 1 , octree);
    children[1] = new OctreeNode(FBoundingBox(FVector(center.x, bounds.min.y, bounds.min.z ),
        FVector(bounds.max.x, center.y, center.z)), depth + 1, octree);
    children[2] = new OctreeNode(FBoundingBox(FVector(bounds.min.x, center.y, bounds.min.z),
        FVector(center.x, bounds.max.y, center.z)), depth + 1, octree);
    children[3] = new OctreeNode(FBoundingBox(FVector(center.x, center.y, bounds.min.z),
        FVector(bounds.max.x, bounds.max.y, center.z)), depth + 1, octree);
    children[4] = new OctreeNode(FBoundingBox(FVector(bounds.min.x, bounds.min.y, center.z),
        FVector(center.x, center.y, bounds.max.z)), depth + 1, octree);
    children[5] = new OctreeNode(FBoundingBox(FVector(center.x, bounds.min.y, center.z),
        FVector(bounds.max.x, center.y, bounds.max.z)), depth + 1, octree);
    children[6] = new OctreeNode(FBoundingBox(FVector(bounds.min.x, center.y, center.z),
        FVector(center.x, bounds.max.y, bounds.max.z)), depth + 1, octree);
    children[7] = new OctreeNode(FBoundingBox(center, bounds.max), depth + 1, octree);
    isLeaf = false;
}

void OctreeNode::Insert(UObject* obj)
{
    if (isLeaf)
    {
        
        objects.push_back(obj);
        octree->objectCount = octree->objectCount + 1;
        if (objects.size() > MAX_OBJECTS && depth < MAX_DEPTH)
        {
            Subdivide();
            octree->objectCount = octree->objectCount - 1;
            // 리프에 있던 객체들을 자식 노드로 재분배
            for (auto it = objects.begin(); it != objects.end(); )
            {
                bool inserted = false;
                for (int i = 0; i < 8; i++)
                {
                    if (children[i]->Contains((*it)->boundingBox))
                    {
                        children[i]->Insert(*it);
                        it = objects.erase(it);
                        inserted = true;
                        break;
                    }
                }
                if (!inserted)
                    ++it;
            }
        }
    }
    else
    {
        bool inserted = false;
        for (int i = 0; i < 8; i++)
        {
            // Contains 함수 바꾸어야 함
            if (children[i]->Contains(obj->boundingBox))
            {
                children[i]->Insert(obj);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            octree->objectCount = octree->objectCount + 1;
            objects.push_back(obj);
        }
    }
}

void OctreeNode::RayCast(const FVector& rayOrigin, const FVector& rayDirection, std::vector<UObject*>& results)
{
    float distance;
    if (!bounds.Intersect(rayOrigin, rayDirection, distance))
        return;

    
    UPrimitiveBatch::GetInstance().AddOctreeRayDetectAABB(bounds, depth);

    // 현재 노드에 저장된 객체들에 대해 교차 판정
    for (auto obj : objects)
    {
        results.push_back(obj);
    }

    // 자식 노드가 있다면 재귀적으로 검사
    if (!isLeaf)
    {
        for (int i = 0; i < 8; i++)
        {
            if (children[i])
                children[i]->RayCast(rayOrigin, rayDirection, results);
        }
    }
}

void OctreeNode::FrustumCull(const FFrustum& frustum, std::vector<UObject*>& visibleObjects)
{
    if (!frustum.IntersectsBox(bounds))
        return;

    for (auto obj : objects)
    {
        if (frustum.IntersectsBox(obj->boundingBox))
            visibleObjects.push_back(obj);
    }

    if (!isLeaf)
    {
        for (int i = 0; i < 8; i++)
        {
            if (children[i])
                children[i]->FrustumCull(frustum, visibleObjects);
        }
    }
}