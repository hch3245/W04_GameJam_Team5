#include "OctreeNode.h"
#include "Vector.h"
#include "Runtime/CoreUObject/UObject/Object.h"
#include "Octree.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "Classes/Engine/StaticMeshActor.h"
#include "UnrealEd/SceneMgr.h"

OctreeNode::OctreeNode(const FBoundingBox& inBounds, int inDepth, Octree* inOctree)
    :bounds(inBounds), isLeaf(true), depth(inDepth), octree(inOctree), holdObjNum(0)
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
    return bounds.BoxContain(box.min, box.max);
}

bool OctreeNode::Intersects(const FBoundingBox& box)
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

    octree->OctreeArray[depth + 1].Add(children[0]);
    octree->OctreeArray[depth + 1].Add(children[1]);
    octree->OctreeArray[depth + 1].Add(children[2]);
    octree->OctreeArray[depth + 1].Add(children[3]);
    octree->OctreeArray[depth + 1].Add(children[4]);
    octree->OctreeArray[depth + 1].Add(children[5]);
    octree->OctreeArray[depth + 1].Add(children[6]);
    octree->OctreeArray[depth + 1].Add(children[7]);
    octree->maxDepth = depth + 1;
}

void OctreeNode::Insert(AStaticMeshActor* obj)
{
    holdObjNum++;
    if (isLeaf)
    {
        objects.push_back(obj);
        if (objects.size() > MAX_OBJECTS && depth < MAX_DEPTH)
        {
            Subdivide();
            // 리프에 있던 객체들을 자식 노드로 재분배
            for (auto it = objects.begin(); it != objects.end(); )
            {
                bool inserted = false;
                for (int i = 0; i < 8; i++)
                {
                    if (children[i]->Intersects((*it)->boundingBox))
                    {
                        children[i]->Insert(*it);
                        inserted = true;
                    }
                }
                if (!inserted)
                    ++it;
                else
                    it = objects.erase(it);
            }
        }
    }
    else
    {
        bool inserted = false;
        for (int i = 0; i < 8; i++)
        {
            // Contains 함수 바꾸어야 함
            if (children[i]->Intersects(obj->boundingBox))
            {
                children[i]->Insert(obj);
                inserted = true;
            }
        }
        if (!inserted) {
            objects.push_back(obj);
        }
    }
}

void OctreeNode::RayCast(const FVector& rayOrigin, const FVector& rayDirection, std::vector<AStaticMeshActor*>& results)
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


void OctreeNode::FrustumCull(const FFrustum& frustum, std::vector<AStaticMeshActor*>& visibleObjects)
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

void OctreeNode::UpdateObjDepthBoundingBox(int inDepth)
{
    if (inDepth == depth) {
        for (auto& depthObj : objects) {
            UPrimitiveBatch::GetInstance().AddOctreeDepthObj(depthObj->boundingBox, depth);
        }
        return;
    }

    if (!isLeaf) {
        for (int i = 0; i < 8; i++)
        {
            if (children[i])
                children[i]->UpdateObjDepthBoundingBox(inDepth);
        }
    }
}

void OctreeNode::GenerateBatches()
{
    TArray<AStaticMeshActor*> meshArray =  GetObjectsIncludeChildren();
        
    // Original OBJ를 만드는 함수
    OriginalOBJBatchIndex = FSceneMgr::BuildStaticBatches(meshArray, 0);

    //// LOD 1 OBJ를 만드는 함수
    //LOD1OBJBatchIndex = FSceneMgr::BuildStaticBatches(meshArray, 1);

    //// LOD 2 OBJ를 만드는 함수
    //LOD2OBJBatchIndex = FSceneMgr::BuildStaticBatches(meshArray, 2);

}

TArray<AStaticMeshActor*> OctreeNode::GetObjectsIncludeChildren()
{
    TArray<AStaticMeshActor*> objArray;
    for (auto& obj : objects) {
        objArray.Add(obj);
    }
    TArray<AStaticMeshActor*> temp;
    
    if (!isLeaf) {
        for (int i = 0; i < 8; i++)
        {
            temp = children[i]->GetObjectsIncludeChildren();
            for (auto& t : temp) {
                objArray.Add(t);
            }
            temp.Empty();
        }
    }
    return objArray;
}

void OctreeNode::GiveOBJBatchIndex(const FVector& cameraPosition, int MaterialNum, int LODLevel, TArray<int>& drawIndexes)
{
    /*FVector nodePosition = (bounds.max + bounds.min) * 0.5f;
    float dist = nodePosition.Distance(cameraPosition);*/

    if (OriginalOBJBatchIndex != -1 && LODLevel == 0) {
        // cameraPositin이 충분히 가깝다면
        // Batch 그리는에 한테 내 OriginalOBJBatchIndex 주면서 Draw하라고 요청
        drawIndexes.Add(OriginalOBJBatchIndex + MaterialNum);
        return;
    }
    //if (LOD1OBJBatchIndex != -1 && LODLevel == 1) {
    //    // cameraPositin이 충분히 가깝다면
    //    if (dist <= LOD1Length && dist > OriginalLength) {
    //        drawIndexes.Add(LOD1OBJBatchIndex + MaterialNum);
    //    }
    //    return;
    // 
    //}

    //if (LOD2OBJBatchIndex != -1 && LODLevel == 2) {
    //    // cameraPositin이 충분히 가깝다면
    //    if (dist > LOD1Length) {
    //        drawIndexes.Add(LOD2OBJBatchIndex + MaterialNum);
    //    }
    //    return;
    //}

    //return;
}
