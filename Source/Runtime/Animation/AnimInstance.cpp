#include "EnginePCH.h"
#include "AnimInstance.h"
#include "Render/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimInstance::InitializeAnimation(USkeletalMeshComponent* InOwner)
{
	OwningComponent = InOwner;
	USkeletalMesh* Mesh = InOwner ? InOwner->GetSkeletalMesh() : nullptr;
	RefSkeleton = Mesh ? &Mesh->RefSkeleton : nullptr;
	PoseContext.RefSkeleton = RefSkeleton;

	NativeInitializeAnimation();
	if (RefSkeleton)
	{
		if (FAnimNode_Base* Root = GetRootNode())
		{
			Root->Initialize(*RefSkeleton);
		}
	}
}

void UAnimInstance::UpdateAnimation(float DeltaTime)
{
	NativeUpdateAnimation(DeltaTime);
	if (FAnimNode_Base* Root = GetRootNode())
	{
		Root->Update({ DeltaTime });
	}
}

void UAnimInstance::EvaluateAnimation(TArray<FTransform>& OutPose)
{
	if (!RefSkeleton)
	{
		return;
	}

	PoseContext.ResetToRefPose();
	if (FAnimNode_Base* Root = GetRootNode())
	{
		Root->Evaluate(PoseContext);
	}
	OutPose = PoseContext.Pose;
}

void UAnimInstance::PlaySequence(UAnimSequence* Sequence, bool bLoop, float PlayRate)
{
	DefaultPlayer.PlayRate = PlayRate;
	DefaultPlayer.SetSequence(Sequence, bLoop);
	if (RefSkeleton)
	{
		DefaultPlayer.Initialize(*RefSkeleton);
	}
}
