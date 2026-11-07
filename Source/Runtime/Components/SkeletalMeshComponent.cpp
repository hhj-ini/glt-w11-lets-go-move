#include "EnginePCH.h"
#include "SkeletalMeshComponent.h"
#include "Asset/AssetManager.h"
#include "Render/SkeletalMesh.h"
#include "Animation/AnimSequence.h"

USkeletalMeshComponent::USkeletalMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetSkeletalMesh(UAssetManager::GetAssetByPath<USkeletalMesh>("Models/AllMight/SK_AllMight_root.fbxbin"));
}

USkeletalMeshComponent::~USkeletalMeshComponent()
{
}

void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	if (!CurrentAnim || !SkeletalMesh)
	{
		return;
	}

	const float Length = CurrentAnim->GetPlayLength();
	CurrentTime += DeltaTime * PlayRate;
	CurrentTime = bLooping ? std::fmod(CurrentTime, Length) : FMath::Min(CurrentTime, Length);
	if (CurrentTime < 0.f)
	{
		CurrentTime += Length;
	}

	for (int32 T = 0; T < TrackToBone.Num(); ++T)
	{
		if (IsValidBoneIndex(TrackToBone[T]))
		{
			BoneSpaceTransforms[TrackToBone[T]] = CurrentAnim->SampleTrack(T, CurrentTime);
		}
	}

	RefreshBoneTransforms();
}

void USkeletalMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	//PlayAnimation(UAssetManager::GetAssetByPath<UAnimSequence>("Imported/ichigo_dance/ANIM_ichigo_dance_Scene.fbxbin"));
}

void USkeletalMeshComponent::SetBoneLocalTransform(int32 BoneIndex, const FTransform& LocalTransform)
{
	EnsurePoseInitialized();
	if (!IsValidBoneIndex(BoneIndex))
	{
		return;
	}

	BoneSpaceTransforms[BoneIndex] = LocalTransform;
	RefreshBoneTransforms();
}

void USkeletalMeshComponent::SetBoneRotation(int32 BoneIndex, const FRotator& Rotation)
{
	EnsurePoseInitialized();
	if (!IsValidBoneIndex(BoneIndex))
	{
		return;
	}

	ApplyBoneRotation(BoneIndex, Rotation);
	RefreshBoneTransforms();
}

void USkeletalMeshComponent::SetBoneRotations(const TArray<std::pair<FString, FRotator>>& BoneRotations)
{
	EnsurePoseInitialized();
	if (!SkeletalMesh)
	{
		return;
	}

	for (const std::pair<FString, FRotator>& BoneRotation : BoneRotations)
	{
		const int32 BoneIndex = FindBoneIndex(BoneRotation.first);
		if (IsValidBoneIndex(BoneIndex))
		{
			ApplyBoneRotation(BoneIndex, BoneRotation.second);
		}
	}
	RefreshBoneTransforms();
}

void USkeletalMeshComponent::ApplyBoneRotation(int32 BoneIndex, const FRotator& Rotation)
{
	// 행벡터 규칙: 먼저 본 원점에서 회전, 그다음 기준 자세(부모까지의 거리 포함)
	// 위치 / 스케일은 기준 자세 그대로 두고 회전만 Rotate * RefRotation으로 합친다 (비균등 스케일이면 근사)
	const FTransform& Ref = SkeletalMesh->RefSkeleton.RefBonePose[BoneIndex];
	const FMatrix Rotate = Rotation.Quaternion().ToFMatrix();
	BoneSpaceTransforms[BoneIndex] = Ref;
	BoneSpaceTransforms[BoneIndex].Rotation = MatrixToRotator(Rotate * Ref.Rotation.Quaternion().ToFMatrix());
}

void USkeletalMeshComponent::ResetToRefPose()
{
	EnsurePoseInitialized();
	if (!SkeletalMesh)
	{
		return;
	}

	BoneSpaceTransforms = SkeletalMesh->RefSkeleton.RefBonePose;
	RefreshBoneTransforms();
}

void USkeletalMeshComponent::RefreshBoneTransforms()
{
	if (!SkeletalMesh)
	{
		return;
	}

	FillComponentSpaceTransforms(BoneSpaceTransforms);
	FinalizeBoneTransform();
}

void USkeletalMeshComponent::PlayAnimation(UAnimSequence* InAnim, bool bInLoop)
{
	EnsurePoseInitialized();

	CurrentAnim = InAnim;
	CurrentTime = 0.0f;
	bLooping = bInLoop;

	TrackToBone.Reset();
	if (!CurrentAnim || !SkeletalMesh)
	{
		return;
	}

	for (const auto& Track : CurrentAnim->Data.Tracks)
	{
		TrackToBone.Add(SkeletalMesh->RefSkeleton.FindBoneIndex(Track.Name.ToString()));
	}
}

void USkeletalMeshComponent::InitPoseFromMesh()
{
	// 임포터가 저장한 기준 자세(FTransform, 부모 상대)를 시작 포즈로 준비한 뒤
	// 부모가 배열 크기와 렌더 메시를 만들고 RefreshBoneTransforms를 부른다
	if (SkeletalMesh)
	{
		BoneSpaceTransforms = SkeletalMesh->RefSkeleton.RefBonePose;
	}
	else
	{
		BoneSpaceTransforms.Reset();
	}

	Super::InitPoseFromMesh();
}
