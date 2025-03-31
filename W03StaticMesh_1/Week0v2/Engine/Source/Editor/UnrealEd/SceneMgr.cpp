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

#include "Core/Math/Octree.h"
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
        World->GetOctree()->GenerateBatches(5);

    }
    catch (const std::exception& e) {
        FString errorMessage = "Error parsing JSON: ";
        errorMessage += e.what();

        UE_LOG(LogLevel::Error, "No Json file");
    }
}

//void FSceneMgr::BuildStaticBatches()
//{
//    CachedRenderData.Empty(); // 기존 데이터 클리어
//
//    // [1] 메시를 머티리얼별로 그룹화 (서브메시 없음)
//    TMap<UMaterial*, TArray<UStaticMeshComponent*>> MaterialGroups;
//    for (AStaticMeshActor* Actor : StaticMeshes)
//    {
//        UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent();
//        if (!MeshComp || !MeshComp->GetStaticMesh())
//            continue;
//
//        // 메시의 모든 머티리얼을 그룹에 추가
//        TArray<FStaticMaterial*> MeshMaterials = MeshComp->GetStaticMesh()->GetMaterials();
//        for (FStaticMaterial* MatSlot : MeshMaterials)
//        {
//            UMaterial* Mat = MatSlot->Material;
//            MaterialGroups.FindOrAdd(Mat).Add(MeshComp);
//        }
//    }
//
//    const int ChunkSize = 100;
//
//    // [2] 머티리얼별로 직접 CachedRenderData 생성
//    for (auto& Pair : MaterialGroups)
//    {
//        UMaterial* Mat = Pair.Key;
//        TArray<UStaticMeshComponent*>& MeshGroup = Pair.Value;
//        int GroupCount = MeshGroup.Num();
//
//        // 청크 단위 처리
//        for (int i = 0; i < GroupCount; i += ChunkSize)
//        {
//            OBJ::FStaticMeshRenderData* pRenderData = new OBJ::FStaticMeshRenderData();
//            pRenderData->Materials.Add(Mat->GetMaterialInfo()); // 머티리얼 정보 저장
//
//            uint32 VertexOffset = 0;
//            int EndIndex = FMath::Min(i + ChunkSize, GroupCount);
//
//            // 각 청크 내 메시 병합
//            for (int j = i; j < EndIndex; ++j)
//            {
//                UStaticMeshComponent* MeshComp = MeshGroup[j];
//                if (!MeshComp || !MeshComp->GetStaticMesh())
//                    continue;
//
//                OBJ::FStaticMeshRenderData* SourceData = MeshComp->GetStaticMesh()->GetRenderData();
//                if (!SourceData)
//                    continue;
//
//                // 월드 변환 적용
//                FMatrix Model = JungleMath::CreateModelMatrix(
//                    MeshComp->GetWorldLocation(),
//                    MeshComp->GetWorldRotation(),
//                    MeshComp->GetWorldScale()
//                );
//
//                // 정점 변환 및 병합
//                TArray<FVertexSimple> TransformedVertices = BakeTransform(SourceData->Vertices, Model);
//                pRenderData->Vertices + TransformedVertices;
//
//                // 인덱스 오프셋 적용
//                for (uint32 idx : SourceData->Indices)
//                {
//                    pRenderData->Indices.Add(idx + VertexOffset);
//                }
//                VertexOffset += TransformedVertices.Num();
//            }
//
//            // 서브셋 설정 (전체를 하나의 서브셋으로 처리)
//            FMaterialSubset Subset;
//            Subset.MaterialIndex = 0; // 단일 머티리얼
//            Subset.IndexStart = 0;
//            Subset.IndexCount = pRenderData->Indices.Num();
//            pRenderData->MaterialSubsets.Add(Subset);
//
//            // GPU 버퍼 생성
//            if (pRenderData->Vertices.Num() > 0)
//            {
//                pRenderData->VertexBuffer = GEngineLoop.renderer.CreateVertexBuffer(
//                    pRenderData->Vertices,
//                    pRenderData->Vertices.Num() * sizeof(FVertexSimple)
//                );
//            }
//
//            if (pRenderData->Indices.Num() > 0)
//            {
//                pRenderData->IndexBuffer = GEngineLoop.renderer.CreateIndexBuffer(
//                    pRenderData->Indices,
//                    pRenderData->Indices.Num() * sizeof(uint32)
//                );
//            }
//
//            //// 바운딩 박스 계산 (옵션)
//            //pRenderData->BoundingBoxMin = CalculateBoundingBoxMin(pRenderData->Vertices);
//            //pRenderData->BoundingBoxMax = CalculateBoundingBoxMax(pRenderData->Vertices);
//
//            CachedRenderData.Add(pRenderData); // 직접 추가
//        }
//    }
//}

int FSceneMgr::BuildStaticBatches(TArray<AStaticMeshActor*> InStaticMeshes, int LOD)
{
    // [1] 메시를 머티리얼별로 그룹화 (서브메시 없음)
    TMap<UMaterial*, TArray<UStaticMeshComponent*>> MaterialGroups;
    for (AStaticMeshActor* Actor : InStaticMeshes)
    {
        UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent();
        if (!MeshComp || !MeshComp->GetStaticMesh())
            continue;

        // 메시의 모든 머티리얼을 그룹에 추가
        TArray<FStaticMaterial*> MeshMaterials = MeshComp->GetStaticMesh()->GetMaterials();
        for (FStaticMaterial* MatSlot : MeshMaterials)
        {
            UMaterial* Mat = MatSlot->Material;
            MaterialGroups.FindOrAdd(Mat).Add(MeshComp);
        }
    }
    // [2] 머티리얼별로 CachedRenderData를 직접 생성 (청크 단위 제거)
    for (auto& Pair : MaterialGroups)
    {
        UMaterial* Mat = Pair.Key;
        TArray<UStaticMeshComponent*>& MeshGroup = Pair.Value;
        int GroupCount = MeshGroup.Num();

        // 하나의 배치로 전체 그룹을 병합합니다.
        OBJ::FStaticMeshRenderData* pRenderData = new OBJ::FStaticMeshRenderData();
        pRenderData->Materials.Add(Mat->GetMaterialInfo()); // 머티리얼 정보 저장

        uint32 VertexOffset = 0;
        int materialindex = 0;
        for (int j = 0; j < GroupCount; ++j)
        {
            UStaticMeshComponent* MeshComp = MeshGroup[j];
            if (!MeshComp || !MeshComp->GetStaticMesh())
                continue;

            switch (LOD)
            {
            case 1:
                if (materialindex == 0)
                    MeshComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Apple_1.obj"));
                else
                    MeshComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"BittenApple_1.obj"));
                break;
            case 2:
                if (materialindex == 0)
                    MeshComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Apple_2.obj"));
                else
                    MeshComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"BittenApple_2.obj"));
                break;
            }

            OBJ::FStaticMeshRenderData* SourceData = MeshComp->GetStaticMesh()->GetRenderData();
            if (!SourceData)
                continue;

            // 월드 변환 적용
            FMatrix Model = JungleMath::CreateModelMatrix(
                MeshComp->GetWorldLocation(),
                MeshComp->GetWorldRotation(),
                MeshComp->GetWorldScale()
            );

            // 정점 변환 및 병합 (Append를 사용)
            TArray<FVertexSimple> TransformedVertices = BakeTransform(SourceData->Vertices, Model);
            pRenderData->Vertices + TransformedVertices;

            // 인덱스 데이터에 오프셋 적용 후 병합
            for (uint32 idx : SourceData->Indices)
            {
                pRenderData->Indices.Add(idx + VertexOffset);
            }
            VertexOffset += TransformedVertices.Num();
        }

        // MaterialSubsets 설정: 전체 배치 데이터를 하나의 subset으로 처리
        FMaterialSubset Subset;
        Subset.MaterialIndex = 0; // 단일 머티리얼 사용
        Subset.IndexStart = 0;
        Subset.IndexCount = pRenderData->Indices.Num();
        pRenderData->MaterialSubsets.Add(Subset);

        // GPU 버퍼 생성
        uint32 vertCount = pRenderData->Vertices.Num();
        if (vertCount > 0)
        {
            pRenderData->VertexBuffer = GEngineLoop.renderer.CreateVertexBuffer(
                pRenderData->Vertices,
                vertCount * sizeof(FVertexSimple)
            );
        }
        else
        {
            pRenderData->VertexBuffer = nullptr;
        }

        uint32 indexCount = pRenderData->Indices.Num();
        if (indexCount > 0)
        {
            pRenderData->IndexBuffer = GEngineLoop.renderer.CreateIndexBuffer(
                pRenderData->Indices,
                indexCount * sizeof(uint32)
            );
        }
        else
        {
            pRenderData->IndexBuffer = nullptr;
        }
        CachedRenderData.Add(pRenderData);
        materialindex++;
    }
    // 시작되는 Index 반환, +1하면 초록색(or 빨간색 일단 모르지만 규칙적으로 들어감)
    return CachedRenderData.Num() - 2;
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

