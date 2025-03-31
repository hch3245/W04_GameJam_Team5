#include "UnrealEd/SceneMgr.h"
#include "JSON/json.hpp"
#include "UObject/Object.h"
#include "Components/SphereComp.h"
#include "Components/CubeComp.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "UObject/ObjectFactory.h"
#include <fstream>
#include "Components/UBillboardComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkySphereComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/Casts.h"

#include "World.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Runtime/Engine/Classes/Engine/FLoaderOBJ.h"

#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/JungleMath.h"
#include "Math/MathUtility.h"
using json = nlohmann::json;

SceneData FSceneMgr::ParseSceneData(const FString& jsonStr)
{
    SceneData sceneData;

    try {
        json j = json::parse(*jsonStr);

        // 버전과 NextUUID 읽기
        sceneData.Version = j["Version"].get<int>();
        sceneData.NextUUID = j["NextUUID"].get<int>();

        // Primitives 처리 (C++14 스타일)
        auto primitives = j["Primitives"];
        for (auto it = primitives.begin(); it != primitives.end(); ++it) {
            int id = std::stoi(it.key());  // Key는 문자열, 숫자로 변환
            const json& value = it.value();
            UObject* obj = nullptr;
            if (value.contains("Type"))
            {
                const FString TypeName = value["Type"].get<std::string>();
                if (TypeName == USphereComp::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<USphereComp>();
                }
                else if (TypeName == UCubeComp::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<UCubeComp>();
                }
                else if (TypeName == UGizmoArrowComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<UGizmoArrowComponent>();
                }
                else if (TypeName == UBillboardComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<UBillboardComponent>();
                }
                else if (TypeName == ULightComponentBase::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<ULightComponentBase>();
                }
                else if (TypeName == USkySphereComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<USkySphereComponent>();
                    USkySphereComponent* skySphere = static_cast<USkySphereComponent*>(obj);
                }
            }

            USceneComponent* sceneComp = static_cast<USceneComponent*>(obj);
            //Todo : 여기다가 Obj Maeh저장후 일기
            //if (value.contains("ObjStaticMeshAsset"))
            if (value.contains("Location")) sceneComp->SetLocation(FVector(value["Location"].get<std::vector<float>>()[0],
                value["Location"].get<std::vector<float>>()[1],
                value["Location"].get<std::vector<float>>()[2]));
            if (value.contains("Rotation")) sceneComp->SetRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
                value["Rotation"].get<std::vector<float>>()[1],
                value["Rotation"].get<std::vector<float>>()[2]));
            if (value.contains("Scale")) sceneComp->SetScale(FVector(value["Scale"].get<std::vector<float>>()[0],
                value["Scale"].get<std::vector<float>>()[1],
                value["Scale"].get<std::vector<float>>()[2]));
            if (value.contains("Type")) {
                UPrimitiveComponent* primitiveComp = Cast<UPrimitiveComponent>(sceneComp);
                if (primitiveComp) {
                    primitiveComp->SetType(value["Type"].get<std::string>());
                }
                else {
                    std::string name = value["Type"].get<std::string>();
                    sceneComp->NamePrivate = name.c_str();
                }
            }
            sceneData.Primitives[id] = sceneComp;
        }

        auto perspectiveCamera = j["PerspectiveCamera"];
        for (auto it = perspectiveCamera.begin(); it != perspectiveCamera.end(); ++it) {
            int id = std::stoi(it.key());  // Key는 문자열, 숫자로 변환
            const json& value = it.value();
            UObject* obj = FObjectFactory::ConstructObject<UCameraComponent>();
            UCameraComponent* camera = static_cast<UCameraComponent*>(obj);
            if (value.contains("Location")) camera->SetLocation(FVector(value["Location"].get<std::vector<float>>()[0],
                    value["Location"].get<std::vector<float>>()[1],
                    value["Location"].get<std::vector<float>>()[2]));
            if (value.contains("Rotation")) camera->SetRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
                value["Rotation"].get<std::vector<float>>()[1],
                value["Rotation"].get<std::vector<float>>()[2]));
            if (value.contains("Rotation")) camera->SetRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
                value["Rotation"].get<std::vector<float>>()[1],
                value["Rotation"].get<std::vector<float>>()[2]));
            if (value.contains("FOV")) camera->SetFOV(value["FOV"].get<float>());
            if (value.contains("NearClip")) camera->SetNearClip(value["NearClip"].get<float>());
            if (value.contains("FarClip")) camera->SetNearClip(value["FarClip"].get<float>());
            
            
            sceneData.Cameras[id] = camera;
        }
    }
    catch (const std::exception& e) {
        FString errorMessage = "Error parsing JSON: ";
        errorMessage += e.what();

        UE_LOG(LogLevel::Error, "No Json file");
    }

    return sceneData;
}

void FSceneMgr::SpawnActorFromSceneData(const FString& jsonStr)
{
    try {
        json j = json::parse(*jsonStr);

        UWorld* World = GEngineLoop.GetWorld();

        // Primitives 처리 (C++14 스타일)
        auto primitives = j["Primitives"];
        for (auto it = primitives.begin(); it != primitives.end(); ++it) {
            int id = std::stoi(it.key());  // Key는 문자열, 숫자로 변환
            const json& value = it.value();
            UObject* obj = nullptr;
            AActor* SpawnedActor = nullptr;
            AStaticMeshActor* StaticMeshActor = nullptr;

            if (value.contains("Type"))
            {
                const FString TypeName = value["Type"].get<std::string>();
                if (TypeName == USphereComp::StaticClass()->GetName())
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT(std::to_string(id) + "(OBJ_SPHERE)"));
                    SpawnedActor->AddComponent<USphereComp>();

                    //obj = FObjectFactory::ConstructObject<USphereComp>();
                }
                else if (TypeName == UCubeComp::StaticClass()->GetName())
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT(std::to_string(id) + "(OBJ_CUBE)"));
                    SpawnedActor->AddComponent<UCubeComp>();

                    //obj = FObjectFactory::ConstructObject<UCubeComp>();
                }
                else if (TypeName == UGizmoArrowComponent::StaticClass()->GetName())
                {
                    // 일단 AActor로 Spawn, 이후 수정 필요할 수 있음
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT(std::to_string(id) + "(OBJ_GIZMO)"));
                    SpawnedActor->AddComponent<UGizmoArrowComponent>();

                    //obj = FObjectFactory::ConstructObject<UGizmoArrowComponent>();
                }
                else if (TypeName == UBillboardComponent::StaticClass()->GetName())
                {
                    // 일단 AActor로 Spawn, 이후 수정 필요할 수 있음
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT(std::to_string(id) + "(OBJ_BILLBOARD)"));
                    SpawnedActor->AddComponent<UBillboardComponent>();

                    //obj = FObjectFactory::ConstructObject<UBillboardComponent>();
                }
                else if (TypeName == ULightComponentBase::StaticClass()->GetName())
                {
                    // 일단 AActor로 Spawn, 이후 수정 필요할 수 있음
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT(std::to_string(id) + "(OBJ_LIGHT)"));
                    SpawnedActor->AddComponent<ULightComponentBase>();

                    obj = FObjectFactory::ConstructObject<ULightComponentBase>();
                }
                else if (TypeName == USkySphereComponent::StaticClass()->GetName())
                {
                    // 일단 AActor로 Spawn, 이후 수정 필요할 수 있음
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT(std::to_string(id) + "(OBJ_SKYSPHERE)"));
                    SpawnedActor->AddComponent<USkySphereComponent>();

                    obj = FObjectFactory::ConstructObject<USkySphereComponent>();
                    USkySphereComponent* skySphere = static_cast<USkySphereComponent*>(obj);
                }
                else if (TypeName == "StaticMeshComp") {
                    // StaticMesh를 쓰는 경우, AStaticMeshActor 을 생성해주어야 함
                    StaticMeshActor = World->SpawnActor<AStaticMeshActor>();
                    StaticMeshActor->SetActorLabel(TEXT(std::to_string(id) + "(OBJ_STATICMESH"));
                }
            }

            USceneComponent* sceneComp = nullptr;
            if (StaticMeshActor != nullptr) {
                sceneComp = StaticMeshActor->GetRootComponent();
            }
            else if (SpawnedActor != nullptr) {
                sceneComp = SpawnedActor->GetRootComponent();
            }

            if (sceneComp != nullptr) {
                if (value.contains("Location")) sceneComp->SetLocation(FVector(value["Location"].get<std::vector<float>>()[0],
                    value["Location"].get<std::vector<float>>()[1],
                    value["Location"].get<std::vector<float>>()[2]));
                if (value.contains("Rotation")) sceneComp->SetRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
                    value["Rotation"].get<std::vector<float>>()[1],
                    value["Rotation"].get<std::vector<float>>()[2]));
                if (value.contains("Scale")) sceneComp->SetScale(FVector(value["Scale"].get<std::vector<float>>()[0],
                    value["Scale"].get<std::vector<float>>()[1],
                    value["Scale"].get<std::vector<float>>()[2]));
                if (value.contains("Type")) {
                    UPrimitiveComponent* primitiveComp = Cast<UPrimitiveComponent>(sceneComp);
                    if (primitiveComp) {
                        primitiveComp->SetType(value["Type"].get<std::string>());
                    }
                    else {
                        std::string name = value["Type"].get<std::string>();
                        sceneComp->NamePrivate = name.c_str();
                    }
                }
            }
            else {
                UE_LOG(LogLevel::Error, "No SceneComponent! in SceneMgr");
            }

            if (StaticMeshActor != nullptr) {
                if (value.contains("ObjStaticMeshAsset"))
                {
                    //Todo : 여기다가 Obj Maeh저장후 일기
                //if (value.contains("ObjStaticMeshAsset"))
                    const FString staticMeshPath = value["ObjStaticMeshAsset"].get<std::string>();
                    UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
                    FManagerOBJ::CreateStaticMesh(staticMeshPath);

                    FString staticMeshName = GetFileNameFromPath(staticMeshPath);
                    auto wStaticMeshName = staticMeshName.ToWideString();
                    MeshComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(wStaticMeshName));
                    StaticMeshes.Add(StaticMeshActor);


                     // 위치랑 Scale, Transform 고려하여 World 좌표계로 변경 MeshComp->AABB
                    FMatrix scaleMatrix = FMatrix::CreateScale(
                        MeshComp->GetWorldScale().x,
                        MeshComp->GetWorldScale().y,
                        MeshComp->GetWorldScale().z
                    );

                    FMatrix rotationMatrix = FMatrix::CreateRotation(
                        MeshComp->GetWorldRotation().x,
                        MeshComp->GetWorldRotation().y,
                        MeshComp->GetWorldRotation().z
                    );


                    FMatrix translationMatrix = FMatrix::CreateTranslationMatrix( MeshComp->GetWorldLocation() * 1.0f);
                    
                    FMatrix worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

                    FVector boundMin = FMatrix::TransformVector(MeshComp->AABB.min, worldMatrix);
                    FVector boundMax = FMatrix::TransformVector(MeshComp->AABB.max, worldMatrix);
                    
                    StaticMeshActor->SetBoundingBox(FBoundingBox(boundMin, boundMax));
                    World->AddOctreeObject(StaticMeshActor);
                    UPrimitiveBatch::GetInstance().AddAABB(StaticMeshActor->boundingBox);
                }
            }

        }

        // 카메라 Spawn
        // 현재는 카메라 1개를 상정하여 World->camera에 해당 값을 바로 전달하겠음
        // 이후 카메라 여러개가 되는 거 할거면 코드 개선해야 하옵니다

        const json& cameraJson = j["PerspectiveCamera"];
        auto ActiveViewportClient = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
        if (cameraJson.contains("Location")) {
            const auto& loc = cameraJson["Location"];
            ActiveViewportClient->ViewTransformPerspective.ViewLocation.x = loc[0];
            ActiveViewportClient->ViewTransformPerspective.ViewLocation.y = loc[1];
            ActiveViewportClient->ViewTransformPerspective.ViewLocation.z = loc[2];
        }
        if (cameraJson.contains("Rotation")) {
            const auto& rot = cameraJson["Rotation"];
            ActiveViewportClient->ViewTransformPerspective.ViewRotation.x = rot[0];
            ActiveViewportClient->ViewTransformPerspective.ViewRotation.y = rot[1];
            ActiveViewportClient->ViewTransformPerspective.ViewRotation.z = rot[2];
        }
        if (cameraJson.contains("FOV")) {
            ActiveViewportClient->ViewFOV = cameraJson["FOV"][0];
        }
        if (cameraJson.contains("NearClip")) {
            ActiveViewportClient->nearPlane = cameraJson["NearClip"][0];
        }
        if (cameraJson.contains("FarClip")) {
            ActiveViewportClient->farPlane = cameraJson["FarClip"][0];
        }


        World->UpdateOctreeFromOctreeobjects();

    }
    catch (const std::exception& e) {
        FString errorMessage = "Error parsing JSON: ";
        errorMessage += e.what();

        UE_LOG(LogLevel::Error, "No Json file");
    }
}

void FSceneMgr::BuildStaticBatches()
{
    StaticBatches.Empty();
    CachedRenderData.Empty();
    TMap<UMaterial*, TArray<UStaticMeshComponent*>> MaterialGroups;
    for (AStaticMeshActor* Actor : StaticMeshes)
    {
        if (!Actor)
            continue;

        UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent();
        if (!MeshComp || !MeshComp->GetStaticMesh())
            continue;

        TArray<FStaticMaterial*> meshMaterials = MeshComp->GetStaticMesh()->GetMaterials();
        if (meshMaterials.Num() <= 0)
            continue;

        UMaterial* Mat = meshMaterials[0]->Material;
        if (!MaterialGroups.Contains(Mat))
        {
            MaterialGroups.Add(Mat, TArray<UStaticMeshComponent*>());
        }
        MaterialGroups[Mat].Add(MeshComp);
    }

    const int ChunkSize = 100;

    for (auto& Pair : MaterialGroups)
    {
        UMaterial* Mat = Pair.Key;
        TArray<UStaticMeshComponent*>& MeshGroup = Pair.Value;
        int GroupCount = MeshGroup.Num();

        // 그룹을 ChunkSize 단위로 나눕니다.
        for (int i = 0; i < GroupCount; i += ChunkSize)
        {
            FStaticBatchData BatchData;
            BatchData.Material = Mat;
            BatchData.CombinedVertices.Empty();
            BatchData.CombinedIndices.Empty();

            uint32 VertexOffset = 0;
            int EndIndex = FMath::Min(i + ChunkSize, GroupCount);
            for (int j = i; j < EndIndex; ++j)
            {
                UStaticMeshComponent* MeshComp = MeshGroup[j];
                if (!MeshComp || !MeshComp->GetStaticMesh())
                    continue;

                OBJ::FStaticMeshRenderData* RenderData = MeshComp->GetStaticMesh()->GetRenderData();
                if (RenderData == nullptr)
                    continue;

                FMatrix Model = JungleMath::CreateModelMatrix(
                    MeshComp->GetWorldLocation(),
                    MeshComp->GetWorldRotation(),
                    MeshComp->GetWorldScale()
                );

                // 위치와 노말 등 정점 데이터 베이크
                TArray<FVertexSimple> TransformedVertices = BakeTransform(RenderData->Vertices, Model);
                BatchData.CombinedVertices + TransformedVertices;

                for (uint32 idx : RenderData->Indices)
                {
                    BatchData.CombinedIndices.Add(idx + VertexOffset);
                }
                VertexOffset += TransformedVertices.Num();
            }

            // 재질 정보 설정: 그룹 내 첫 번째 메쉬의 StaticMesh에서 Materials 배열을 그대로 복사
            if (MeshGroup.Num() > 0 && MeshGroup[0]->GetStaticMesh())
            {
                BatchData.Materials = MeshGroup[0]->GetStaticMesh()->GetMaterials();
            }
            else
            {
                BatchData.Materials.Empty();
            }

            // MaterialSubsets 설정: 전체 배치 데이터를 하나의 subset으로 처리
            FMaterialSubset Subset;
            Subset.MaterialIndex = 0;
            Subset.IndexStart = 0;
            Subset.IndexCount = BatchData.CombinedIndices.Num();
            BatchData.MaterialSubsets.Add(Subset);

            // StaticBatches에 저장 (여기서는 GPU 버퍼는 아직 생성하지 않음)
            StaticBatches.Add(BatchData);
        }
    }
    for (const FStaticBatchData& Batch : StaticBatches)
    {
        OBJ::FStaticMeshRenderData* pRenderData = new OBJ::FStaticMeshRenderData();

        pRenderData->Vertices = Batch.CombinedVertices;
        pRenderData->Indices = Batch.CombinedIndices;

        pRenderData->MaterialSubsets = Batch.MaterialSubsets;
        uint32 vertCount = pRenderData->Vertices.Num();
        if (vertCount > 0)
        {
            pRenderData->VertexBuffer = GEngineLoop.renderer.CreateVertexBuffer(
                pRenderData->Vertices,
                vertCount * sizeof(FVertexSimple)  // 전체 바이트 크기
            );
        }
        else
        {
            pRenderData->VertexBuffer = nullptr;
        }

        // GPU 버퍼 생성: IndexBuffer
        uint32 indexCount = pRenderData->Indices.Num();
        if (indexCount > 0)
        {
            pRenderData->IndexBuffer = GEngineLoop.renderer.CreateIndexBuffer(
                pRenderData->Indices,
                indexCount * sizeof(uint32)        // 전체 바이트 크기
            );
        }
        else
        {
            pRenderData->IndexBuffer = nullptr;
        }
        CachedRenderData.Add(pRenderData);
    }
}

TArray<FVertexSimple> FSceneMgr::BakeTransform(const TArray<FVertexSimple>& sourceVertices, const FMatrix& transform)
{
    TArray<FVertexSimple> transformedVertices;
    transformedVertices.Reserve(sourceVertices.Num());

    for (const FVertexSimple& v : sourceVertices)
    {
        FVertexSimple tv = v; // 원본 정점 데이터를 복사합니다.

        // 위치 변환: FMatrix::TransformVector를 사용하여 (x, y, z) 좌표에 변환을 적용합니다.
        FVector4 pos(v.x, v.y, v.z, 1.f);
        // transform.TransformVector()를 호출하여 새 위치를 얻습니다.
        FVector4 newPos = FMatrix::TransformVector(pos, transform);
        tv.x = newPos.x;
        tv.y = newPos.y;
        tv.z = newPos.z;

        // 노말 변환: 동일한 행렬을 사용해 변환한 후, 정규화합니다.
        // (비균일 스케일링이 있는 경우 inverse-transpose 행렬을 사용해야 합니다.)
        FVector normal(v.nx, v.ny, v.nz);
        FVector newNormal = FMatrix::TransformVector(normal, transform);
        newNormal.Normalize();
        tv.nx = newNormal.x;
        tv.ny = newNormal.y;
        tv.nz = newNormal.z;

        // 색상, 텍스처 좌표(u,v), MaterialIndex 등 나머지 속성은 그대로 유지합니다.
        transformedVertices.Add(tv);
    }

    return transformedVertices;
}

std::string FSceneMgr::GetFileNameFromPath(const FString& Path)
{
    std::string path = GetData(Path);
    // 경로에서 마지막 '\' 또는 '/'의 위치를 찾습니다.
    size_t pos = path.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        // 슬래시 다음부터 끝까지 반환
        return path.substr(pos + 1);
    }
    // 슬래시가 없으면 전체 문자열이 파일 이름으로 간주됩니다.
    return path;
}

FString FSceneMgr::LoadSceneFromFile(const FString& filename)
{
    std::ifstream inFile(*filename);
    if (!inFile) {
        UE_LOG(LogLevel::Error, "Failed to open file for reading: %s", *filename);
        return FString();
    }

    json j;
    try {
        inFile >> j; // JSON 파일 읽기
    }
    catch (const std::exception& e) {
        UE_LOG(LogLevel::Error, "Error parsing JSON: %s", e.what());
        return FString();
    }

    inFile.close();

    return j.dump(4);
}

std::string FSceneMgr::SerializeSceneData(const SceneData& sceneData)
{
    json j;

    // Version과 NextUUID 저장
    j["Version"] = sceneData.Version;
    j["NextUUID"] = sceneData.NextUUID;

    // Primitives 처리 (C++17 스타일)
    for (const auto& [Id, Obj] : sceneData.Primitives)
    {
        USceneComponent* primitive = static_cast<USceneComponent*>(Obj);
        std::vector<float> Location = { primitive->GetWorldLocation().x,primitive->GetWorldLocation().y,primitive->GetWorldLocation().z };
        std::vector<float> Rotation = { primitive->GetWorldRotation().x,primitive->GetWorldRotation().y,primitive->GetWorldRotation().z };
        std::vector<float> Scale = { primitive->GetWorldScale().x,primitive->GetWorldScale().y,primitive->GetWorldScale().z };

        std::string primitiveName = *primitive->GetName();
        size_t pos = primitiveName.rfind('_');
        if (pos != INDEX_NONE) {
            primitiveName = primitiveName.substr(0, pos);
        }
        j["Primitives"][std::to_string(Id)] = {
            {"Location", Location},
            {"Rotation", Rotation},
            {"Scale", Scale},
            {"Type",primitiveName}
        };
    }

    for (const auto& [id, camera] : sceneData.Cameras)
    {
        UCameraComponent* cameraComponent = static_cast<UCameraComponent*>(camera);
        TArray<float> Location = { cameraComponent->GetWorldLocation().x, cameraComponent->GetWorldLocation().y, cameraComponent->GetWorldLocation().z };
        TArray<float> Rotation = { 0.0f, cameraComponent->GetWorldRotation().y, cameraComponent->GetWorldRotation().z };
        float FOV = cameraComponent->GetFOV();
        float nearClip = cameraComponent->GetNearClip();
        float farClip = cameraComponent->GetFarClip();
    
        //
        j["PerspectiveCamera"][std::to_string(id)] = {
            {"Location", Location},
            {"Rotation", Rotation},
            {"FOV", FOV},
            {"NearClip", nearClip},
            {"FarClip", farClip}
        };
    }


    return j.dump(4); // 4는 들여쓰기 수준
}

bool FSceneMgr::SaveSceneToFile(const FString& filename, const SceneData& sceneData)
{
    std::ofstream outFile(*filename);
    if (!outFile) {
        FString errorMessage = "Failed to open file for writing: ";
        MessageBoxA(NULL, *errorMessage, "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    std::string jsonData = SerializeSceneData(sceneData);
    outFile << jsonData;
    outFile.close();

    return true;
}

void FSceneMgr::BuildDynamicBatches(const std::vector<UObject*>& visibleObjects)
{
    StaticBatches.Empty();
    CachedRenderData.Empty();

    TMap<UMaterial*, TArray<UStaticMeshComponent*>> MaterialGroups;

    // 가시성이 확인된 오브젝트만 처리
    for (UObject* obj : visibleObjects)
    {
        AStaticMeshActor* Actor = Cast<AStaticMeshActor>(obj);
        if (!Actor)
            continue;

        UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent();
        if (!MeshComp || !MeshComp->GetStaticMesh())
            continue;

        TArray<FStaticMaterial*> meshMaterials = MeshComp->GetStaticMesh()->GetMaterials();
        if (meshMaterials.Num() <= 0)
            continue;

        UMaterial* Mat = meshMaterials[0]->Material;
        if (!MaterialGroups.Contains(Mat))
        {
            MaterialGroups.Add(Mat, TArray<UStaticMeshComponent*>());
        }
        MaterialGroups[Mat].Add(MeshComp);
    }

    // 공간적 근접성에 따라 배치 생성
    const int ChunkSize = 50;

    for (auto& Pair : MaterialGroups)
    {
        UMaterial* Mat = Pair.Key;
        TArray<UStaticMeshComponent*>& MeshGroup = Pair.Value;

        // 공간적으로 근접한 오브젝트끼리 그룹화
        SortByProximity(FVector(0.0f, 0.0f, 0.0f), MeshGroup);

        int GroupCount = MeshGroup.Num();
        for (int i = 0; i < GroupCount; i += ChunkSize)
        {
            FStaticBatchData BatchData;
            BatchData.Material = Mat;
            BatchData.CombinedVertices.Empty();
            BatchData.CombinedIndices.Empty();

            uint32 VertexOffset = 0;
            int EndIndex = FMath::Min(i + ChunkSize, GroupCount);
            for (int j = i; j < EndIndex; ++j)
            {
                UStaticMeshComponent* MeshComp = MeshGroup[j];
                if (!MeshComp || !MeshComp->GetStaticMesh())
                    continue;

                OBJ::FStaticMeshRenderData* RenderData = MeshComp->GetStaticMesh()->GetRenderData();
                if (RenderData == nullptr)
                    continue;

                FMatrix Model = JungleMath::CreateModelMatrix(
                    MeshComp->GetWorldLocation(),
                    MeshComp->GetWorldRotation(),
                    MeshComp->GetWorldScale()
                );

                // 위치와 노말 등 정점 데이터 베이크
                TArray<FVertexSimple> TransformedVertices = BakeTransform(RenderData->Vertices, Model);
                for (uint32 i = 0; i < TransformedVertices.Num(); i++)
                {
                    BatchData.CombinedVertices.Add(TransformedVertices[i]);
                }

                for (uint32 idx : RenderData->Indices)
                {
                    BatchData.CombinedIndices.Add(idx + VertexOffset);
                }
                VertexOffset += TransformedVertices.Num();
            }

            if (MeshGroup.Num() > 0 && MeshGroup[0]->GetStaticMesh())
            {
                BatchData.Materials = MeshGroup[0]->GetStaticMesh()->GetMaterials();
            }

            FMaterialSubset Subset;
            Subset.MaterialIndex = 0;
            Subset.IndexStart = 0;
            Subset.IndexCount = BatchData.CombinedIndices.Num();
            BatchData.MaterialSubsets.Add(Subset);

            StaticBatches.Add(BatchData);
        }
    }

    CreateGPUBuffers();
}

// 공간적 근접성에 따라 메시 정렬
void FSceneMgr::SortByProximity(FVector ReferencePoint, TArray<UStaticMeshComponent*>& MeshGroup)
{
    if (MeshGroup.Num() <= 1)
        return;

    // 거리에 따라 정렬
    MeshGroup.Sort([ReferencePoint](const UStaticMeshComponent* A, const UStaticMeshComponent* B) {
        float distA = ReferencePoint.Distance(A->GetLocalLocation());
        float distB = ReferencePoint.Distance(B->GetLocalLocation());
        return distA < distB;
    });
}

// GPU 버퍼 생성
void FSceneMgr::CreateGPUBuffers()
{
    for (const FStaticBatchData& Batch : StaticBatches)
    {
        OBJ::FStaticMeshRenderData* pRenderData = new OBJ::FStaticMeshRenderData();

        pRenderData->Vertices = Batch.CombinedVertices;
        pRenderData->Indices = Batch.CombinedIndices;
        pRenderData->MaterialSubsets = Batch.MaterialSubsets;

        uint32 vertCount = pRenderData->Vertices.Num();
        if (vertCount > 0)
        {
            pRenderData->VertexBuffer = GEngineLoop.renderer.CreateVertexBuffer(
                pRenderData->Vertices,
                vertCount * sizeof(FVertexSimple)
            );
        }

        uint32 indexCount = pRenderData->Indices.Num();
        if (indexCount > 0)
        {
            pRenderData->IndexBuffer = GEngineLoop.renderer.CreateIndexBuffer(
                pRenderData->Indices,
                indexCount * sizeof(uint32)
            );
        }

        CachedRenderData.Add(pRenderData);
    }
}