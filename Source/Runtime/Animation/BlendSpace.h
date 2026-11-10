#pragma once
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "Math/Vector.h"

class UAnimSequence;

struct FBlendSample
{
	UAnimSequence* Animation = nullptr;
	FVector SampleValue;
};

class UBlendSpace : public UObject
{
	DECLARE_CLASS(UBlendSpace, UObject)
public:
	void AddSample(UAnimSequence* Anim, const FVector& Value);
	const TArray<FBlendSample>& GetSamples() const { return SampleData; }

protected:
	TArray<FBlendSample> SampleData;
};

class UBlendSpace1D : public UBlendSpace
{
	DECLARE_CLASS(UBlendSpace1D, UBlendSpace)

public:
	bool GetSamplesToBlend(float X, int32& OutI0, int32& OutI1, float& OutAlpha) const;
};