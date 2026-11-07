#pragma once
#include "Asset/RenderAsset.h"
#include "AnimSequenceTypes.h"
#include "Math/Transform.h"

class USkeleton;

class UAnimSequence : public URenderAsset
{
	DECLARE_CLASS(UAnimSequence, URenderAsset)

public:
	USkeleton* Skeleton = nullptr;
	FAnimSequenceData Data;

	float GetPlayLength() const { return Data.SequenceLength; }

	// Bone의 인덱스와 시간을 받아서 샘플링
	FTransform SampleTrack(int32 TrackIndex, float Time) const
	{
		const FRawAnimSequenceTrack& K = Data.Tracks[TrackIndex].InternalTrack;
		const int32 N = K.PosKeys.Num();
		if (N == 1)
		{
			return FTransform(K.RotKeys[0].ToFRotator(), K.PosKeys[0], K.ScaleKeys[0]);
		}

		const float Frame = FMath::Clamp(Time * Data.FrameRate, 0.f, static_cast<float>(N - 1));
		const int32 F0 = static_cast<int32>(Frame);
		const int32 F1 = FMath::Min(F0 + 1, N - 1);
		const float A = Frame - F0;

		const FVector Pos = K.PosKeys[F0] + (K.PosKeys[F1] - K.PosKeys[F0]) * A;
		const FVector Scale = K.ScaleKeys[F0] + (K.ScaleKeys[F1] - K.ScaleKeys[F0]) * A;

		const FQuat& Q0 = K.RotKeys[F0];
		FQuat Q1 = K.RotKeys[F1];

		const FQuat Rot = FQuat::NLerp(Q0, Q1, A);

		return FTransform(Rot.ToFRotator(), Pos, Scale);
	}
};