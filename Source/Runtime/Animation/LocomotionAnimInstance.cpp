#include "EnginePCH.h"
#include "LocomotionAnimInstance.h"
#include "BlendSpace.h"
#include "Asset/AssetManager.h"

ULocomotionAnimInstance::~ULocomotionAnimInstance() = default;

void ULocomotionAnimInstance::NativeInitializeAnimation()
{
	if (BlendSpace)
	{
		return;
	}

	auto* Idle = UAssetManager::GetAssetByPath<UAnimSequence>("Imported/UAL1_Standard/ANIM_UAL1_Standard_Armature_Idle_Loop.fbxbin");
	auto* Walk = UAssetManager::GetAssetByPath<UAnimSequence>("Imported/UAL1_Standard/ANIM_UAL1_Standard_Armature_Walk_Loop.fbxbin");
	auto* Jog = UAssetManager::GetAssetByPath<UAnimSequence>("Imported/UAL1_Standard/ANIM_UAL1_Standard_Armature_Jog_Fwd_Loop.fbxbin");
	auto* Sprint = UAssetManager::GetAssetByPath<UAnimSequence>("Imported/UAL1_Standard/ANIM_UAL1_Standard_Armature_Sprint_Loop.fbxbin");

	BlendSpace.reset(NewObject<UBlendSpace1D>(this));
	BlendSpace->AddSample(Idle, FVector(0.0f, 0, 0));
	BlendSpace->AddSample(Walk, FVector(150.0f, 0, 0));
	BlendSpace->AddSample(Jog, FVector(350.0f, 0, 0));
	BlendSpace->AddSample(Sprint, FVector(600.0f, 0, 0));

	LocomotionNode.BlendSpace = BlendSpace.get();
}

void ULocomotionAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	TestTime += DeltaTime;
	Speed = (std::sin(TestTime * 0.5f) * 0.5f + 0.5f) * 600.0f;
	LocomotionNode.X = Speed;
}
