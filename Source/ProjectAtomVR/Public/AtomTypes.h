#pragma once

#include "Engine.h"
#include "AtomTypes.generated.h"

UENUM()
enum class EHand : uint8
{
	Left,
	Right
};

UENUM()
enum class EHandType : uint8
{
	Dominate,
	Nondominate
};

USTRUCT()
struct PROJECTATOMVR_API FHandAnim
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Anim)
	class UAnimSequence* Right = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = Anim)
	class UAnimSequence* Left = nullptr;
};

//-----------------------------------------------------------------
// Information used when spawning a decal in-game.
//-----------------------------------------------------------------
USTRUCT()
struct PROJECTATOMVR_API FDecalInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	class UMaterial* Material = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FVector DecalSize = FVector{ 20.0f, 20.0f, 20.0f };

	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 1.0f;
};

USTRUCT()
struct PROJECTATOMVR_API FMotionTransformRep
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector_NetQuantize10 Location;
	
	UPROPERTY()
	FRotator Rotation;
};