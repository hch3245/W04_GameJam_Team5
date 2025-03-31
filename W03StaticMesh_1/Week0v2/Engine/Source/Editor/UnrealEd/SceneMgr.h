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

class FSceneMgr
{
public:
    static SceneData ParseSceneData(const FString& jsonStr);
    static void SpawnActorFromSceneData(const FString& jsonStr);
    static void BuildStaticBatches();
    static int BuildStaticBatches(TArray<AStaticMeshActor*> StaticMeshes);
    static TArray<FVertexSimple> BakeTransform(const TArray<FVertexSimple>& sourceVertices, const FMatrix& transform);
    static std::string GetFileNameFromPath(const FString& Path);
    static FString LoadSceneFromFile(const FString& filename);
    static std::string SerializeSceneData(const SceneData& sceneData);
    static bool SaveSceneToFile(const FString& filename, const SceneData& sceneData);

    static const TArray<OBJ::FStaticMeshRenderData*>& GetCachedRenderData() { return CachedRenderData; }
    inline static TArray<AStaticMeshActor*> StaticMeshes;
    inline static TArray<OBJ::FStaticMeshRenderData*> CachedRenderData;
};

