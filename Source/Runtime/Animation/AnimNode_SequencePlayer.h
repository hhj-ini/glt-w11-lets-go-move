#pragma once
#include "Animation/AnimNodeBase.h"

class UAnimSequence;

struct FAnimNode_SequencePlayer : public FAnimNode_Base
{
	UAnimSequence* Sequence = nullptr;
	float PlayRate = 1.0f;
	bool bLoop = true;

	void SetSequence(UAnimSequence* InSequence, bool bInLoop = true);
	float GetCurrentAnimNodeTime() const { return CurrentTime; }

	virtual void Initialize(const FReferenceSkeleton& RefSkeleton) override;
	virtual void Update(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate(FPoseContext& Output) override;

private:
	void BuildTrackMap();

	float CurrentTime = 0.0f;
	const FReferenceSkeleton* CachedSkeleton = nullptr;
	TArray<int32> TrackToBone;
};