#pragma once
#include "Math/Transform.h"
#include "ReferenceSkeleton.h"

class UAnimSequence;

struct FAnimationRuntime
{
	static FTransform BlendTransform(const FTransform& A, const FTransform& B, float Alpha);

	static void BlendTwoPosesTogether(const TArray<FTransform>& PoseA, const TArray<FTransform>& PoseB,
		float WeightOfA, TArray<FTransform>& OutPose);

	static void BuildTrackToBone(const UAnimSequence* Seq, const FReferenceSkeleton& Skel, TArray<int32>& OutMap);

	static void SamplePose(const UAnimSequence* Seq, const TArray<int32>& TrackToBone, float Time, TArray<FTransform>& InOutPose);

	static float WrapTime(float Time, float Length, bool bLoop);
};