// Copyright 2016 Epic Wolf Productions, Inc. All Rights Reserved.

#pragma once

#include "Sound/SoundNode.h"
#include "SoundNodeFadeOut.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTATOMVR_API USoundNodeFadeOut : public USoundNode
{
	GENERATED_BODY()
	
	/** USoundNode Interface Begin */
public:

	virtual int32 GetMaxChildNodes() const override { return 2; }
	virtual int32 GetMinChildNodes() const override { return 2; }
	virtual int32 GetNumSounds(const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound) const override { return 1; }
	virtual float GetDuration(void) override;
	virtual void CreateStartingConnectors(void) override;
	virtual void ParseNodes(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const struct FSoundParseParameters& ParseParams, TArray<FWaveInstance *>& WaveInstances) override;

#if WITH_EDITOR
	virtual FText GetInputPinName(int32 PinIndex) const override;
#endif
	/** USoundNode Interface End */			
};
