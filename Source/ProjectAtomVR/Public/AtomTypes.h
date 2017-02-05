#pragma once

#include "Engine.h"
#include "AtomTypes.generated.h"

UENUM()
enum class EHand : uint8
{
	Left,
	Right
};

FORCEINLINE constexpr EHand operator !(EHand Hand)
{
	return static_cast<EHand>(!static_cast<__underlying_type(EHand)>(Hand));
}

UENUM()
enum class EHandType : uint8
{
	Dominate,
	Nondominate
};

FORCEINLINE constexpr EHandType operator !(EHandType HandType)
{
	return static_cast<EHandType>(!static_cast<__underlying_type(EHandType)>(HandType));
}

USTRUCT()
struct PROJECTATOMVR_API FHandAnim
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Anim)
	class UAnimSequence* DetachedLeft = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = Anim)
	class UAnimSequence* DetachedRight = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = Anim)
	class UAnimMontage* FullBodyLeft = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = Anim)
	class UAnimMontage* FullBodyRight = nullptr;
};

//-----------------------------------------------------------------
// Information used when spawning a decal in-game.
//-----------------------------------------------------------------
USTRUCT()
struct PROJECTATOMVR_API FDecalInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	class UMaterialInterface* Material = nullptr;

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

UENUM()
enum class ELoadoutType : uint8
{
	Item,
	Weapon,
	Ability
};

UENUM()
enum class ECharacterClass : uint8
{
	Assault,
	Defense,
	Repair,
	Support
};

// Unique handle that can be used to distinguish help indicators that have been set.
USTRUCT(BlueprintType)
struct FHelpIndicatorHandle
{
	GENERATED_BODY()

	friend class AVRHUD;

	FHelpIndicatorHandle()
		: Handle(0)
	{
	}

	bool IsValid() const
	{
		return Handle != 0;
	}

	void Reset()
	{
		Handle = 0;
	}

	bool operator==(const FHelpIndicatorHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FHelpIndicatorHandle& Other) const
	{
		return Handle != Other.Handle;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%ull"), Handle);
	}

private:
	UPROPERTY(Transient)
	uint64 Handle;
};