#pragma once

#include "Containers/Array.h"
#include "Math/Vector.h"
#include "Core/NameTypes.h"
#include "Math/Quat.h"

struct FRawAnimSequenceTrack
{
	// 부모공간 기준
	TArray<FVector> PosKeys;	// 위치
	TArray<FQuat> RotKeys;		// 회전
	TArray<FVector> ScaleKeys;	// 크기
};

struct FBoneAnimationTrack
{
	FName Name; // 뼈 이름
	FRawAnimSequenceTrack InternalTrack;
};

struct FAnimSequenceData
{
	FString StackName;
	float SequenceLength = 0.f;
	float FrameRate = 30.f;
	int32 NumFrames = 0;
	TArray<FBoneAnimationTrack> Tracks;
};

