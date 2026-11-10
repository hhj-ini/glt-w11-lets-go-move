#include "EnginePCH.h"
#include "AnimationRuntime.h"
#include "AnimSequence.h"

FTransform FAnimationRuntime::BlendTransform(const FTransform& A, const FTransform& B, float Alpha)
{
	const FVector Loc = A.Location + (B.Location - A.Location) * Alpha;
	const FVector Scale = A.Scale + (B.Scale - A.Scale) * Alpha;
	const FQuat Rot = FQuat::SLerp(A.GetOrientation(), B.GetOrientation(), Alpha);
	return FTransform(Rot.ToFRotator(), Loc, Scale);
}

void FAnimationRuntime::BlendTwoPosesTogether(const TArray<FTransform>& PoseA, const TArray<FTransform>& PoseB, float WeightOfA, TArray<FTransform>& OutPose)
{
	const float Alpha = 1.0f - WeightOfA;
	OutPose.SetNum(PoseA.Num());	
	for (int32 i = 0; i < OutPose.Num(); ++i)
	{
		OutPose[i] = BlendTransform(PoseA[i], PoseB[i], Alpha);
	}

}

void FAnimationRuntime::BuildTrackToBone(const UAnimSequence* Seq, const FReferenceSkeleton& Skel, TArray<int32>& OutMap)
{
	OutMap.Reset();

	if (!Seq)
	{
		return;
	}

	for (const auto& Track : Seq->Data.Tracks)
	{
		OutMap.Add(Skel.FindBoneIndex(Track.Name.ToString()));
	}
}

void FAnimationRuntime::SamplePose(const UAnimSequence* Seq, const TArray<int32>& TrackToBone, float Time, TArray<FTransform>& InOutPose)
{
	if (!Seq)
	{
		return;
	}

	for (int32 T = 0; T < TrackToBone.Num(); ++T)
	{
		const int32 Bone = TrackToBone[T];
		if (Bone >= 0 && Bone < InOutPose.Num())
		{
			InOutPose[Bone] = Seq->SampleTrack(T, Time);
		}
	}
}

float FAnimationRuntime::WrapTime(float Time, float Length, bool bLoop)
{
	if (Length <= 0.0f)
	{
		return 0.0f;
	}

	if (!bLoop)
	{
		return FMath::Clamp(Time, 0.0f, Length);
	}

	Time = std::fmod(Time, Length);
	return Time < 0.0f ? Time + Length : Time;
}
