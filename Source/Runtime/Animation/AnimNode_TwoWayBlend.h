#pragma once
#include "AnimNodeBase.h"
#include "AnimationRuntime.h"

struct FAnimNode_TwoWayBlend : public FAnimNode_Base
{
	FPoseLink A;
	FPoseLink B;
	float Alpha = 0.0f;

	virtual void Initialize(const FReferenceSkeleton& RefSkeleton) override
	{
		A.Initialize(RefSkeleton);
		B.Initialize(RefSkeleton);
	}

	virtual void Update(const FAnimationUpdateContext& Context) override
	{
		A.Update(Context);
		B.Update(Context);
	}

	virtual void Evaluate(FPoseContext& Output) override
	{
		const float W = FMath::Clamp(Alpha, 0.0f, 1.0f);
		if (W <= 0.0f)
		{
			A.Evaluate(Output);
			return;
		}

		if (W >= 0.0f)
		{
			B.Evaluate(Output);
			return;
		}

		FPoseContext PoseA(Output);
		FPoseContext PoseB(Output);
		A.Evaluate(PoseA);
		B.Evaluate(PoseB);

		FAnimationRuntime::BlendTwoPosesTogether(PoseA.Pose, PoseB.Pose, 1.0f - W, Output.Pose);
	}
};