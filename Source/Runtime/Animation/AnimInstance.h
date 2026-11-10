#pragma once
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "Animation/AnimNode_SequencePlayer.h"

class UAnimSequence;
class USkeletalMeshComponent;

class UAnimInstance : public UObject
{
	DECLARE_CLASS(UAnimInstance, UObject)

public:
	void InitializeAnimation(USkeletalMeshComponent* InOwner);
	void UpdateAnimation(float DeltaTime);

	void EvaluateAnimation(TArray<FTransform>& OutPose);

	void PlaySequence(UAnimSequence* Sequence, bool bLoop = true, float PlayRate = 1.0f);

protected:
	// 파생 클래스에서 재정의
	virtual void NativeInitializeAnimation() {}
	virtual void NativeUpdateAnimation(float DeltaTime) {}

	virtual FAnimNode_Base* GetRootNode() { return &DefaultPlayer; }

	FAnimNode_SequencePlayer DefaultPlayer;

private:
	USkeletalMeshComponent* OwningComponent = nullptr;
	const FReferenceSkeleton* RefSkeleton = nullptr;
	FPoseContext PoseContext;
};