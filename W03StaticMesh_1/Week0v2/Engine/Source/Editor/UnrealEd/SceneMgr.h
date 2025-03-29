#pragma once
#include "Define.h"
#include "Container/Map.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"

class UObject;
struct SceneData {
    int32 Version;
    int32 NextUUID;
    TMap<int32, UObject*> Primitives;
    TMap<int32, UObject*> Cameras;
};
struct FStaticBatchData
{
    UMaterial* Material;                    // 그룹화 기준 머티리얼
    TArray<FStaticMaterial*> Materials;       // 머티리얼 정보 배열 (예: StaticMesh의 Materials)
    TArray<FMaterialSubset> MaterialSubsets;  // 서브셋 정보
    TArray<FVertexSimple> CombinedVertices;
    TArray<uint32> CombinedIndices;
};

class FSceneMgr
{
public:
    static SceneData ParseSceneData(const FString& jsonStr);
    static void SpawnActorFromSceneData(const FString& jsonStr);
    static void BuildStaticBatches();
    static TArray<FVertexSimple> BakeTransform(const TArray<FVertexSimple>& sourceVertices, const FMatrix& transform);
    static std::string GetFileNameFromPath(const FString& Path);
    static FString LoadSceneFromFile(const FString& filename);
    static std::string SerializeSceneData(const SceneData& sceneData);
    static bool SaveSceneToFile(const FString& filename, const SceneData& sceneData);

    static const TArray<FStaticBatchData>& GetStaticBatches() { return StaticBatches; }
    static const TArray<OBJ::FStaticMeshRenderData*>& GetCachedRenderData() { return CachedRenderData; }
    inline static TArray<AStaticMeshActor*> StaticMeshes;
    inline static TArray<FStaticBatchData> StaticBatches;
    inline static TArray<OBJ::FStaticMeshRenderData*> CachedRenderData;
};

