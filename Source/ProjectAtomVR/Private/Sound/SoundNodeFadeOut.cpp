// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#include "ProjectAtomVR.h"
#include "SoundNodeFadeOut.h"
#include "ActiveSound.h"
#include "Platform.h"


void USoundNodeFadeOut::CreateStartingConnectors()
{
	InsertChildNode(ChildNodes.Num());
	InsertChildNode(ChildNodes.Num());
}

void USoundNodeFadeOut::ParseNodes(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const struct FSoundParseParameters& ParseParams, TArray<FWaveInstance *>& WaveInstances)
{
	USoundNode* const ChildNode = ActiveSound.bFadingOut ? ChildNodes[1] : ChildNodes[0];

	const UPTRINT ChildWaveInstanceHash = GetNodeWaveInstanceHash(NodeWaveInstanceHash, ChildNode, 0);
	ChildNode->ParseNodes(AudioDevice, ChildWaveInstanceHash, ActiveSound, ParseParams, WaveInstances);
}

float USoundNodeFadeOut::GetDuration(void)
{
	check(ChildNodes.Num() > 0);
	return ChildNodes[0]->GetDuration();
}

#if WITH_EDITOR
FText USoundNodeFadeOut::GetInputPinName(int32 PinIndex) const
{
	return (PinIndex == 0) ? FText::FromName(TEXT("Main")) : FText::FromName(TEXT("FadeOut"));
}
#endif