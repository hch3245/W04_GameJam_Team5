#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"

#include "UClass.h"
#include "UObjectHash.h"
#include "Runtime/Launch/Define.h"

UClass* UObject::StaticClass()
{
    static UClass ClassInfo{TEXT("UObject"), sizeof(UObject), alignof(UObject), nullptr};
    return &ClassInfo;
}

void UObject::SetBoundingBox(const FVector& boundMin, const FVector& boundMax)
{
    boundingBox.min = boundMin;
    boundingBox.max = boundMax;
}

void UObject::SetBoundingBox(const FBoundingBox& inBoundingBox)
{
    boundingBox = inBoundingBox;
}


UObject::UObject()
    : UUID(0)
    // TODO: Object를 생성할 때 직접 설정하기
    , InternalIndex(std::numeric_limits<uint32>::max())
    , NamePrivate("None")
{
}

bool UObject::IsA(const UClass* SomeBase) const
{
    const UClass* ThisClass = GetClass();
    return ThisClass->IsChildOf(SomeBase);
}
