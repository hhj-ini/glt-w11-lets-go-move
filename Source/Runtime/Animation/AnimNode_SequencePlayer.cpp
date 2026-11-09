#include "EnginePCH.h"
#include "AnimNode_SequencePlayer.h"
#include "AnimSequence.h"

void FAnimNode_SequencePlayer::SetSequence(UAnimSequence* InSequence, bool bInLoop)
{
	Sequence = InSequence;
	bLoop = bInLoop;
	CurrentTime = 0.0f;
	BuildTrackMap();
}

void FAnimNode_SequencePlayer::Initialize(const FReferenceSkeleton& RefSkeleton)
{
	CachedSkeleton = &RefSkeleton;
	BuildTrackMap();
}

void FAnimNode_SequencePlayer::Update(const FAnimationUpdateContext& Context)
{
	if (!Sequence)
	{
		return;
	}

	const float Length = Sequence->GetPlayLength();
	if (Length <= 0.0f)
	{
		return;
	}

	CurrentTime += Context.DeltaTime * PlayRate;
	if (bLoop)
	{
		CurrentTime = std::fmod(CurrentTime, Length);
		if (CurrentTime < 0.f)
		{
			CurrentTime += Length;	
		}
	}
	else
	{
		CurrentTime = FMath::Clamp(CurrentTime, 0.f, Length);
	}
}

void FAnimNode_SequencePlayer::Evaluate(FPoseContext& Output)
{
	if (!Sequence)
	{
		return;
	}

	for (int32 T = 0; T < TrackToBone.Num(); ++T)
	{
		const int32 Bone = TrackToBone[T];
		if (Bone >= 0 && Bone < Output.Pose.Num())
		{
			Output.Pose[Bone] = Sequence->SampleTrack(T, CurrentTime);
		}
	}
}

void FAnimNode_SequencePlayer::BuildTrackMap()
{
	TrackToBone.Reset();
	if (!Sequence || !CachedSkeleton)
	{
		return;
	}

	for (const auto& Track : Sequence->Data.Tracks)
	{
		TrackToBone.Add(CachedSkeleton->FindBoneIndex(Track.Name.ToString()));
	}
}
