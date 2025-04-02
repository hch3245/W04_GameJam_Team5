#pragma once

#include "Runtime/CoreUObject/UObject/Object.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

// 헤더만 만들어 둠, 현재 기능 업음
class ULevel : public UObject
{
public:
    const TArray<AActor*>& GetActors() const { return Actors; }
private:
    TArray<AActor*> Actors;
};

