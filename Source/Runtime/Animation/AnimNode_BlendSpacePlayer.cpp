#include "EnginePCH.h"
#include "AnimNode_BlendSpacePlayer.h"
#include "BlendSpace.h"
#include "AnimSequence.h"
#include "AnimationRuntime.h"

void FAnimNode_BlendSpacePlayer::Initialize(const FReferenceSkeleton& Skel)
{
	TrackMaps.Reset();
	if (!BlendSpace)
	{
		return;
	}

	const auto& Samples = BlendSpace->GetSamples();
	TrackMaps.SetNum(Samples.Num());

	for (int32 i = 0; i < Samples.Num(); ++i)
	{
		FAnimationRuntime::BuildTrackToBone(Samples[i].Animation, Skel, TrackMaps[i]);
	}

}

void FAnimNode_BlendSpacePlayer::Update(const FAnimationUpdateContext& Context)
{
	if (!BlendSpace || !BlendSpace->GetSamplesToBlend(X, I0, I1, Alpha))
	{
		I0 = I1 = -1;
		return;
	}

	const auto& Samples = BlendSpace->GetSamples();
	const float L0 = Samples[I0].Animation->GetPlayLength();
	const float L1 = Samples[I1].Animation->GetPlayLength();
	const float Length = L0 + (L1 - L0) * Alpha;
	if (Length <= 0.0f)
	{
		return;
	}

	NormalizedTime = FAnimationRuntime::WrapTime(NormalizedTime + Context.DeltaTime * PlayRate / Length, 1.0f, bLoop);
}

void FAnimNode_BlendSpacePlayer::Evaluate(FPoseContext& Output)
{
	if (I0 < 0 || I0 >= TrackMaps.Num() || I1 >= TrackMaps.Num())
	{
		return;
	}

	const auto& Samples = BlendSpace->GetSamples();

	auto SampleAt = [&](int32 i, TArray<FTransform>& Pose)
	{
		const UAnimSequence* Seq = Samples[i].Animation;
		FAnimationRuntime::SamplePose(Seq, TrackMaps[i], NormalizedTime * Seq->GetPlayLength(), Pose);
	};

	if (I0 == I1 || Alpha <= 0.0f)
	{
		SampleAt(I0, Output.Pose);
		return;
	}

	if (Alpha >= 1.0f)
	{
		SampleAt(I1, Output.Pose);
		return;
	}

	FPoseContext PoseA(Output);
	FPoseContext PoseB(Output);

	SampleAt(I0, PoseA.Pose);
	SampleAt(I1, PoseB.Pose);

	FAnimationRuntime::BlendTwoPosesTogether(PoseA.Pose, PoseB.Pose, 1.0f - Alpha, Output.Pose);
}
