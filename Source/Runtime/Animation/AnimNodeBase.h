#pragma once
#include "Animation/ReferenceSkeleton.h"
#include "Math/Transform.h"

struct FAnimationUpdateContext
{
	float DeltaTime = 0.f;
};

// Evaluate 결과 포즈
struct FPoseContext
{
	const FReferenceSkeleton* RefSkeleton = nullptr;
	TArray<FTransform> Pose;

	void ResetToRefPose() { Pose = RefSkeleton->RefBonePose; }
};


struct FAnimNode_Base
{
	virtual ~FAnimNode_Base() = default;

	virtual void Initialize(const FReferenceSkeleton& RefSkeleton) {}
	virtual void Update(const FAnimationUpdateContext& Context) = 0;
	virtual void Evaluate(FPoseContext& Output) = 0;
};

struct FPoseLink
{
	FAnimNode_Base* LinkNode = nullptr;

	void Initialize(const FReferenceSkeleton& Skel)
	{
		if (LinkNode)
		{
			LinkNode->Initialize(Skel);
		}
	}

	void Update(const FAnimationUpdateContext& Ctx)
	{
		if (LinkNode)
		{
			LinkNode->Update(Ctx);
		}
	}

	void Evaluate(FPoseContext& Output)
	{
		if (LinkNode)
		{
			LinkNode->Evaluate(Output);
		}
	}
};