// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CharacterSelectWidget.generated.h"

class AAtomCharacterSelect;
class UPanelWidget;
class UTextBlock;
enum class ECharacterClass : uint8;

/**
 * Widget to allow the player to select a specified character.
 */
UCLASS()
class PROJECTATOMVR_API UCharacterSelectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UCharacterSelectWidget(const FObjectInitializer& ObjectInitializer);

	void SetOwner(AAtomCharacterSelect* InOwner);

	AAtomCharacterSelect* GetOwner() const;

protected:
	UFUNCTION(BlueprintCallable, Category = CharacterSelectUI)
	void OnCharacterSelected();

	FText GetClassEnumText(ECharacterClass CharacterClass) const;

	class UCharacterSelectTextWidget* CreateTextWidget(const FText& Text) const;

	/** UUserWidget Interface Begin */
	virtual void NativeConstruct() override;
	/** UUserWidget Interface End */	

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText; // Displays the character name

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ClassText; // Displays the character class

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* WeaponsPanel; // Panel for weapon text widgets

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* AbilitiesPanel; // Panel for ability text widgets

	UPROPERTY(EditDefaultsOnly, Category = CharacterSelectUI)
	TSubclassOf<class UCharacterSelectTextWidget> TextWidgetClass;

private:
	UPROPERTY(Transient, BlueprintReadOnly, Category = CharacterSelectUI, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AAtomCharacterSelect> Owner;	
};
