// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "CharacterSelectWidget.h"
#include "TextBlock.h"
#include "AtomLoadoutTemplate.h"
#include "AtomLoadout.h"
#include "AtomCharacter.h"
#include "CharacterSelectTextWidget.h"
#include "PanelWidget.h"




UCharacterSelectWidget::UCharacterSelectWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UCharacterSelectWidget::SetOwner(AAtomCharacterSelect* InOwner)
{
	check(InOwner);

	Owner = InOwner;
}

AAtomCharacterSelect* UCharacterSelectWidget::GetOwner() const
{
	return Owner.Get();
}

void UCharacterSelectWidget::OnCharacterSelected()
{	
	check(Owner.IsValid());

	if (TSubclassOf<AAtomCharacter> Character = Owner->GetCharacterClass())
	{
		UGameInstance* GameInstance = GetWorld()->GetGameInstance();
		if (AAtomPlayerController* Controller = Cast<AAtomPlayerController>(GameInstance->GetFirstLocalPlayerController()))
		{
			Controller->ServerRequestCharacterChange(Character);
		}
	}
}

FText UCharacterSelectWidget::GetClassEnumText(ECharacterClass CharacterClass) const
{
	switch (CharacterClass)
	{
	case ECharacterClass::Assault:
		return FText::FromName(TEXT("Assault"));
	case ECharacterClass::Defense:
		return FText::FromName(TEXT("Defense"));
	case ECharacterClass::Support:
		return FText::FromName(TEXT("Support"));
	case ECharacterClass::Repair:
		return FText::FromName(TEXT("Repair"));
	default:
		return FText::FromName(TEXT("None"));
	}
}

void UCharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	check(Owner.IsValid());

	if (Owner->GetCharacterClass() != nullptr && TextWidgetClass != nullptr)
	{
		AAtomCharacter* CharacterCDO = Owner->GetCharacterClass()->GetDefaultObject<AAtomCharacter>();

		FString CharacterName = CharacterCDO->GetClass()->GetName();
		UAtomUtilsFunctionLibrary::TrimClassName(CharacterName);
		NameText->SetText(FText::FromString(CharacterName));
		ClassText->SetText(GetClassEnumText(CharacterCDO->GetCharacterClass()));

		const UAtomLoadout* const Loadout = CharacterCDO->GetLoadout();
		const UAtomLoadoutTemplate* const LoadoutTemplate = Loadout->GetLoadoutTemplate()->GetDefaultObject<UAtomLoadoutTemplate>();

		const TArray<FAtomLoadoutTemplateSlot>& TemplateSlots = LoadoutTemplate->GetLoadoutSlots();

		// Get all weapons and abilities into separate lists.
		TArray<TPair<FName, uint32>, TInlineAllocator<4>> Weapons; // Name and count for each weapon
		TArray<FName, TInlineAllocator<4>> Abilities;

		for (const FAtomLoadoutTemplateSlot& LoadoutSlot : TemplateSlots)
		{
			if (LoadoutSlot.ItemClass != nullptr)
			{
				const AAtomEquippable* const Item = LoadoutSlot.ItemClass->GetDefaultObject<AAtomEquippable>();

				if (Item->GetLoadoutType() == ELoadoutType::Weapon)
				{
					auto Entry = Weapons.FindByPredicate([LoadoutSlot](TPair<FName, uint32>& Weapon) { return Weapon.Key == LoadoutSlot.ItemClass->GetFName(); });

					if (Entry != nullptr)
					{
						++Entry->Value;
					}
					else
					{						
						Weapons.Emplace(TPairInitializer<FName, uint32>(LoadoutSlot.ItemClass->GetFName(), 1));
					}
				}
				else if (Item->GetLoadoutType() == ELoadoutType::Ability)
				{
					Abilities.Add(LoadoutSlot.ItemClass->GetFName());
				}
			}			
		}

		// Add entries into the weapon panel
		for (const auto& WeaponPair : Weapons)
		{
			FString WeaponText = WeaponPair.Key.ToString();
			UAtomUtilsFunctionLibrary::TrimClassName(WeaponText);

			if (WeaponPair.Value > 1)
			{
				WeaponText.Append(FString::Printf(TEXT(" x %d"), WeaponPair.Value));
			}

			UCharacterSelectTextWidget* WeaponEntry = CreateTextWidget(FText::FromString(WeaponText));
			WeaponsPanel->AddChild(WeaponEntry);
		}

		// Add entries into the ability panel
		for (const auto& Ability : Abilities)
		{
			FString AbilityText = Ability.ToString();
			UAtomUtilsFunctionLibrary::TrimClassName(AbilityText);

			AbilitiesPanel->AddChild(CreateTextWidget(FText::FromString(AbilityText)));
		}
	}
}

UCharacterSelectTextWidget* UCharacterSelectWidget::CreateTextWidget(const FText& Text) const
{
	UCharacterSelectTextWidget* TextWidget = CreateWidget<UCharacterSelectTextWidget>(GetWorld(), TextWidgetClass);
	TextWidget->SetPlayerContext(GetPlayerContext());
	TextWidget->SetText(Text);

	return TextWidget;
}
