// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AICharacter_Base.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAICharacter_Base::AAICharacter_Base()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_AnimatedMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Animated Mesh");
	_AnimatedMesh->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AAICharacter_Base::BeginPlay()
{
	Super::BeginPlay();
	GEngine->AddOnScreenDebugMessage(952, 1.f, FColor::Red, "RHLF: TICK");


	/*if (_TestAnim)
	{
		_AnimMaxDT = _TestAnim->GetPlayLength();
		GEngine->AddOnScreenDebugMessage(865, 1.f, FColor::Red, "Anim Length: " + FString::SanitizeFloat(_AnimMaxDT));

	}*/
}

// Called every frame
void AAICharacter_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(952, 1.f, FColor::Red, "RHLF: TICK");

	PhysicsAnimationTick(DeltaTime);
}

// Called to bind functionality to input
void AAICharacter_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAICharacter_Base::PhysicsAnimationTick(float DeltaTime)
{	
	/*{
		int BoneIndex = GetMesh()->GetBoneIndex("hand_r");
		FTransform BoneT = GetMesh()->GetBoneTransform(BoneIndex);
		GEngine->AddOnScreenDebugMessage(862, INFINITY, FColor::Green, "MeshPose: || Location: " + BoneT.GetLocation().ToString()
			+ " || Rotation: " + BoneT.GetRotation().ToString() + " || Scale: " + BoneT.GetScale3D().ToString());

		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		if (!AnimInst)
		{
			GEngine->AddOnScreenDebugMessage(861, 10, FColor::Red, "OutPose: AnimInst NULL");
			return;
		}

		//FAnimMontageInstance* AnimMon = AnimInst->GetActiveMontageInstance();
		if (!_TestAnim)
		{
			GEngine->AddOnScreenDebugMessage(861, 10.f, FColor::Red, "OutPose: AnimMon NULL");
			return;
		}

		FCompactPose OutPose;
		OutPose.SetBoneContainer(&AnimInst->GetRequiredBones());
		FBlendedCurve OutCurve;
		FStackCustomAttributes OutAttributes;
		FAnimationPoseData OutPoseData(OutPose, OutCurve, OutAttributes);
		FAnimExtractContext AnimExtract(DeltaTime);

		_TestAnim->GetAnimationPose(OutPoseData, AnimExtract);

		_AnimDT += DeltaTime;
		GEngine->AddOnScreenDebugMessage(865, INFINITY, FColor::Green, "Anim DT: " + FString::SanitizeFloat(_AnimDT));
		if (_AnimDT > _AnimMaxDT)
			_AnimDT = 0;

		const TArray<FTransform>& BonesTransforms = reinterpret_cast<const TArray<FTransform>&>(OutPose.GetBones());
		const FReferenceSkeleton& Skel = _TestAnim->GetSkeleton()->GetReferenceSkeleton();
		int32 BoneI = Skel.FindBoneIndex("hand_r");
		FTransform AnimBoneT = BonesTransforms[BoneI];
		FTransform MeshT = GetMesh()->GetComponentTransform();
		GEngine->AddOnScreenDebugMessage(861, INFINITY, FColor::Green, "OutPose: || Location: " +
			(AnimBoneT.GetLocation() + MeshT.GetLocation()).ToString()
			+ " || Rotation: " + (MeshT.GetRotation() * AnimBoneT.GetRotation()).ToString()
			+ " || Scale: " + AnimBoneT.GetScale3D().ToString());
	}*/

	TArray<FTransform> BoneTransforms;
	int32 NumBones = _AnimatedMesh->GetNumBones();
	for (int i = 0; i < NumBones; i++)
	{
		BoneTransforms.Add(_AnimatedMesh->GetBoneTransform(i));
	}

	EditBoneTransform_Physics(DeltaTime, BoneTransforms);
	//GetMesh()->AddForce(FVector(1000, 0, 2500), "hand_r", true);
}

void AAICharacter_Base::EditBoneTransform_NonPhysics(TArray<FTransform>& BoneTransforms)
{

}

void AAICharacter_Base::EditBoneTransform_Physics(float DeltaTime, TArray<FTransform>& BoneTransforms)
{
	GEngine->AddOnScreenDebugMessage(952, 1.f, FColor::Red, "RHLF: NULL");

	if (!_AnimatedMesh || !GetMesh())
		return;

	//right hand
	int32 RightHandBoneIndex = _AnimatedMesh->GetBoneIndex(_RightHandBoneName);
	FTransform RightHandBoneTransform = _AnimatedMesh->GetBoneTransform(RightHandBoneIndex);
	
	FVector RightHandBoneLocation = RightHandBoneTransform.GetLocation();
	FQuat RightHandBoneQuat = RightHandBoneTransform.GetRotation();

	FVector RightHandLinearForce = _RH_LocPD.GetForce(DeltaTime, GetMesh()->GetBoneTransform(RightHandBoneIndex).GetLocation(),
		RightHandBoneLocation);

	FVector RightHandAngularForce = _RH_RotPD.GetTorque(DeltaTime, GetMesh()->GetBoneTransform(RightHandBoneIndex).GetRotation(),
		RightHandBoneQuat, GetMesh()->GetPhysicsAngularVelocityInRadians(_RightHandBoneName), FVector::ZeroVector, RightHandBoneTransform);

	GEngine->AddOnScreenDebugMessage(952, 1.f, FColor::Red, _RightHandBoneName.ToString() + ": " + RightHandLinearForce.ToString());

	GetMesh()->AddForce(RightHandLinearForce, _RightHandBoneName, true);

	//right elbow
	int32 RightElbowBoneIndex = _AnimatedMesh->GetBoneIndex(_RightElbowBoneName);
	FTransform RightElbowBoneTransform = _AnimatedMesh->GetBoneTransform(RightElbowBoneIndex);
	
	FVector RightElbowBoneLocation = RightElbowBoneTransform.GetLocation();
	FQuat RightElbowBoneQuat = RightElbowBoneTransform.GetRotation();

	FVector RightElbowLinearForce = _RE_LocPD.GetForce(DeltaTime, GetMesh()->GetBoneTransform(RightElbowBoneIndex).GetLocation(),
		RightElbowBoneLocation);

	GEngine->AddOnScreenDebugMessage(953, 1.f, FColor::Red, _RightElbowBoneName.ToString() + ": " + RightElbowLinearForce.ToString());

	GetMesh()->AddForce(RightElbowLinearForce, _RightElbowBoneName, true);


	//right shoulder
	int32 RightShoulderBoneIndex = _AnimatedMesh->GetBoneIndex(_RightShoulderBoneName);
	FTransform RightShoulderBoneTransform = _AnimatedMesh->GetBoneTransform(RightShoulderBoneIndex);
	
	FVector RightShoulderBoneLocation = RightShoulderBoneTransform.GetLocation();
	FQuat RightShoulderBoneQuat = RightShoulderBoneTransform.GetRotation();

	FVector RightShoulderLinearForce = _RS_LocPD.GetForce(DeltaTime, GetMesh()->GetBoneTransform(RightShoulderBoneIndex).GetLocation(),
		RightShoulderBoneLocation);

	GEngine->AddOnScreenDebugMessage(954, 1.f, FColor::Red, _RightShoulderBoneName.ToString() + ": " + RightShoulderLinearForce.ToString());

	GetMesh()->AddForce(RightShoulderLinearForce, _RightShoulderBoneName, true);
}

