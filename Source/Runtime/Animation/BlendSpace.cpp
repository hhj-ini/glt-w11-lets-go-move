#include "EnginePCH.h"
#include "BlendSpace.h"

void UBlendSpace::AddSample(UAnimSequence* Anim, const FVector& Value)
{
	if (!Anim)
	{
		return;
	}

	SampleData.Add({ Anim, Value });

	for (int32 i = SampleData.Num() - 1; i > 0 && SampleData[i - 1].SampleValue.X > SampleData[i].SampleValue.X; --i)
	{
		std::swap(SampleData[i - 1], SampleData[i]);
	}
}


bool UBlendSpace1D::GetSamplesToBlend(float X, int32& OutI0, int32& OutI1, float& OutAlpha) const
{
	const int32 N = SampleData.Num();
	if (N == 0)
	{
		return false;
	}

	if (N == 1 || X <= SampleData[0].SampleValue.X)
	{
		OutI0 = OutI1 = 0;
		OutAlpha = 0.0f;
		return true;
	}

	if (X >= SampleData[N - 1].SampleValue.X)
	{
		OutI0 = OutI1 = N - 1;
		OutAlpha = 0.0f;
		return true;
	}

	for (int32 i = 0; i < N - 1; ++i)
	{
		const float X0 = SampleData[i].SampleValue.X;
		const float X1 = SampleData[i + 1].SampleValue.X;

		if (X <= X1)
		{
			OutI0 = i;
			OutI1 = i + 1;

			OutAlpha = (X1 > X0) ? (X - X0) / (X1 - X0) : 0.0f;
			return true;
		}
	}
	return false;
}

