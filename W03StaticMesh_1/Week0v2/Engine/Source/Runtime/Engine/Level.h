#pragma once

#include "Runtime/CoreUObject/UObject/Object.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

class Octree;

// 헤더만 만들어 둠, 현재 기능 업음
class ULevel : public UObject
{
    DECLARE_CLASS(ULevel, UObject)

public:
    ULevel() = default;

    FString LevelName;
    UWorld* OwningWorld;

    void Initialize();
    void Tick(float DeltaTime);
    void Release();

    // 액터 관리 함수들
    template <typename T>
        requires std::derived_from<T, AActor>
    T* SpawnActor();

    bool DestroyActor(AActor* ThisActor);

    const TSet<AActor*>& GetActors() const { return ActorsArray; }

    void CreateBaseObject();
    void ReleaseBaseObject();

private:
    // 액터 관리 변수들
    TSet<AActor*> ActorsArray;
    TArray<AActor*> PendingBeginPlayActors;

    // 옥트리 관련 변수들
    Octree* LevelOctree = nullptr;
    TArray<UObject*> OctreeObjects;
};