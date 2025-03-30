#include "Engine/Source/Runtime/Engine/World.h"

#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Engine/FLoaderOBJ.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SkySphereComponent.h"

#include "Runtime/Core/Math/Octree.h"
#include "Runtime/Core/Math/OctreeNode.h"


void UWorld::Initialize()
{
    // TODO: Load Scene
    CreateBaseObject();
    //SpawnObject(OBJ_CUBE);
    //FManagerOBJ::CreateStaticMesh("Assets/Dodge/Dodge.obj");

    //FManagerOBJ::CreateStaticMesh("Assets/SkySphere.obj");
    //AActor* SpawnedActor = SpawnActor<AActor>();
    //USkySphereComponent* skySphere = SpawnedActor->AddComponent<USkySphereComponent>();
    //skySphere->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"SkySphere.obj"));
    //skySphere->GetStaticMesh()->GetMaterials()[0]->Material->SetDiffuse(FVector((float)32/255, (float)171/255, (float)191/255));

    CreateOctree(); // Octree의 크기를 최대 범위로 만든 뒤에 수정
}

void UWorld::CreateBaseObject()
{
    if (EditorPlayer == nullptr)
    {
        EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>();;
    }

    if (camera == nullptr)
    {
        camera = FObjectFactory::ConstructObject<UCameraComponent>();
        camera->SetLocation(FVector(8.0f, 8.0f, 8.f));
        camera->SetRotation(FVector(0.0f, 45.0f, -135.0f));
    }

    if (LocalGizmo == nullptr)
    {
        LocalGizmo = FObjectFactory::ConstructObject<UTransformGizmo>();
    }
}

void UWorld::ReleaseBaseObject()
{
    if (LocalGizmo)
    {
        delete LocalGizmo;
        LocalGizmo = nullptr;
    }

    if (worldGizmo)
    {
        delete worldGizmo;
        worldGizmo = nullptr;
    }

    if (camera)
    {
        delete camera;
        camera = nullptr;
    }

    if (EditorPlayer)
    {
        delete EditorPlayer;
        EditorPlayer = nullptr;
    }
    
    if (worldOctree) 
    {
        delete worldOctree;
        worldOctree = nullptr;
    }

}

void UWorld::Tick(float DeltaTime)
{
	camera->TickComponent(DeltaTime);
	EditorPlayer->Tick(DeltaTime);
	LocalGizmo->Tick(DeltaTime);

    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    for (AActor* Actor : PendingBeginPlayActors)
    {
        Actor->BeginPlay();
    }
    PendingBeginPlayActors.Empty();

    // 매 틱마다 Actor->Tick(...) 호출
	for (AActor* Actor : ActorsArray)
	{
	    Actor->Tick(DeltaTime);
	}
}

void UWorld::Release()
{
	for (AActor* Actor : ActorsArray)
	{
		Actor->EndPlay(EEndPlayReason::WorldTransition);
        TSet<UActorComponent*> Components = Actor->GetComponents();
	    for (UActorComponent* Component : Components)
	    {
	        GUObjectArray.MarkRemoveObject(Component);
	    }
	    GUObjectArray.MarkRemoveObject(Actor);
	}
    ActorsArray.Empty();

	pickingGizmo = nullptr;
	ReleaseBaseObject();

    GUObjectArray.ProcessPendingDestroyObjects();
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }

    // 액터의 Destroyed 호출
    ThisActor->Destroyed();

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    TSet<UActorComponent*> Components = ThisActor->GetComponents();
    for (UActorComponent* Component : Components)
    {
        Component->DestroyComponent();
    }

    // World에서 제거
    ActorsArray.Remove(ThisActor);

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}

void UWorld::MakeOctree(FVector boundMin, FVector boundMax)
{
    FBoundingBox boundBox = { boundMin, boundMax };
    worldOctree = new Octree(boundBox);
}

void UWorld::CreateOctree()
{
    MakeOctree(FVector(FLT_MAX, FLT_MAX, FLT_MAX), FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX));
}

void UWorld::AddOctreeObject(UObject* OctreeObject)
{
    OctreeObjects.Add(OctreeObject);
    if (worldOctree->root->bounds.min.x > OctreeObject->boundingBox.min.x)
        worldOctree->root->bounds.min.x = OctreeObject->boundingBox.min.x;
    if (worldOctree->root->bounds.min.y > OctreeObject->boundingBox.min.y)
        worldOctree->root->bounds.min.y = OctreeObject->boundingBox.min.y;
    if (worldOctree->root->bounds.min.z > OctreeObject->boundingBox.min.z)
        worldOctree->root->bounds.min.z = OctreeObject->boundingBox.min.z;
    
    if (worldOctree->root->bounds.max.x < OctreeObject->boundingBox.max.x)
        worldOctree->root->bounds.max.x = OctreeObject->boundingBox.max.x;
    if (worldOctree->root->bounds.max.y < OctreeObject->boundingBox.max.y)
        worldOctree->root->bounds.max.y = OctreeObject->boundingBox.max.y;
    if (worldOctree->root->bounds.max.z < OctreeObject->boundingBox.max.z)
        worldOctree->root->bounds.max.z = OctreeObject->boundingBox.max.z;
}

void UWorld::UpdateOctreeFromOctreeobjects()
{
    for (auto& octreeObject : OctreeObjects) 
    {
        worldOctree->Insert(octreeObject);
    }
    OctreeObjects.Empty();
}

void UWorld::SetPickingGizmo(UObject* Object)
{
	pickingGizmo = Cast<USceneComponent>(Object);
}