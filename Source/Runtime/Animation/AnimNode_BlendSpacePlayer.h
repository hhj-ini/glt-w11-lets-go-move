#pragma once
#include "AnimNodeBase.h"

class UBlendSpace1D;

struct FAnimNode_BlendSpacePlayer : public FAnimNode_Base
{
	UBlendSpace1D* BlendSpace = nullptr;
	float X = 0.0f;
	float PlayRate = 1.0f;
	bool bLoop = true;

	virtual void Initialize(const FReferenceSkeleton& Skel) override;
	virtual void Update(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate(FPoseContext& Output) override;

private:
	float NormalizedTime = 0.0f;
	int32 I0 = -1;
	int32 I1 = -1;
	float Alpha = 0.0f;
	TArray<TArray<int32>> TrackMaps;
};