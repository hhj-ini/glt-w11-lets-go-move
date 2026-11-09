#pragma once
#include "AnimInstance.h"
#include "AnimNode_BlendSpacePlayer.h"
#include "BlendSpace.h"
#include "AnimSequence.h"

class ULocomotionAnimInstance : public UAnimInstance
{
	DECLARE_CLASS(ULocomotionAnimInstance, UAnimInstance)

public:
	virtual ~ULocomotionAnimInstance() override;

	float Speed = 0.0f;
	float WalkSpeed = 150.0f;
	float RunSpeed = 400.0f;

protected:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	virtual FAnimNode_Base* GetRootNode() override { return &LocomotionNode; } 

private:
	TUniquePtr<UBlendSpace1D> BlendSpace;
	FAnimNode_BlendSpacePlayer LocomotionNode;
	float TestTime = 0.0f;
};