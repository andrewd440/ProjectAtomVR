// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "AtomGameUserSettings.h"

namespace
{
	constexpr auto AtomPlayerSettingsSection = TEXT("/Script/ProjectAtomVR.AtomPlayerSettings");
	constexpr auto PlayerHeightKey = TEXT("PlayerHeight");
	constexpr auto IsRightHandedKey = TEXT("IsRightHanded");

	constexpr auto LoadoutOffsetFormatKey = TEXT("%s_LoadoutOffset");
}

FAtomPlayerSettings& UAtomGameUserSettings::GetPlayerSettings()
{
	return PlayerSettings;
}

void UAtomGameUserSettings::LoadSettings(bool bForceReload /*= false*/)
{
	Super::LoadSettings(bForceReload);

	LoadPlayerSettings();
}

void UAtomGameUserSettings::LoadPlayerSettings()
{
	check(GConfig);
	
	GConfig->GetFloat(AtomPlayerSettingsSection, PlayerHeightKey, PlayerSettings.PlayerHeight, GGameUserSettingsIni);

	bool bIsRightHanded;
	GConfig->GetBool(AtomPlayerSettingsSection, IsRightHandedKey, bIsRightHanded, GGameUserSettingsIni);
	PlayerSettings.bIsRightHanded = bIsRightHanded;

	for (auto& CharacterSetting : PlayerSettings.CharacterSettings)
	{
		check(CharacterSetting.Character);
		FString CharacterName = CharacterSetting.Character->GetName();
		UAtomUtilsFunctionLibrary::TrimClassName(CharacterName);

		GConfig->GetFloat(AtomPlayerSettingsSection, *FString::Printf(LoadoutOffsetFormatKey, *CharacterName),
			CharacterSetting.LoadoutOffset, GGameUserSettingsIni);
	}
}

void UAtomGameUserSettings::SaveSettings()
{
	Super::SaveSettings();

	SavePlayerSettings();
}

void UAtomGameUserSettings::SavePlayerSettings()
{
	check(GConfig);

	GConfig->SetFloat(AtomPlayerSettingsSection, PlayerHeightKey, PlayerSettings.PlayerHeight, GGameUserSettingsIni);
	GConfig->SetBool(AtomPlayerSettingsSection, IsRightHandedKey, PlayerSettings.bIsRightHanded, GGameUserSettingsIni);

	for (auto& CharacterSetting : PlayerSettings.CharacterSettings)
	{
		check(CharacterSetting.Character);
		FString CharacterName = CharacterSetting.Character->GetName();
		UAtomUtilsFunctionLibrary::TrimClassName(CharacterName);

		GConfig->SetFloat(AtomPlayerSettingsSection, *FString::Printf(LoadoutOffsetFormatKey, *CharacterName),
			CharacterSetting.LoadoutOffset, GGameUserSettingsIni);
	}
}
