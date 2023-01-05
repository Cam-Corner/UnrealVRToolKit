// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Utility/PIDControllers.h"
#include "AICharacter_Base.generated.h"


class USkeletalMeshComponent;

struct PIDControllerData
{
	float _PGain = 0;
	float _IGain = 0;
	float _DGain = 0;
};

USTRUCT(BlueprintType)
struct FPhysicsBoneData
{
	GENERATED_BODY()
public:
	FPhysicsBoneData() {}

	FPhysicsBoneData(PIDControllerData LinearData, PIDControllerData AngularData, 
		FName Bone_Name) /*: _Bone_Name(Bone_Name)*/
	{
		_LinearController._PGain = LinearData._PGain;
		_LinearController._IGain = LinearData._IGain;
		_LinearController._DGain = LinearData._DGain;

		_AngularController._PGain = AngularData._PGain;
		_AngularController._DGain = AngularData._DGain;
	}

	void GetBonePhysicsUpdate(FVector& FinalLinearForce, FVector& FinalAngularForce)
	{

	}

	UPROPERTY()
	FName _Bone_Name = NAME_None;

	UPROPERTY()
	FPIDVectorController _LinearController;

	UPROPERTY()
	FQuatPDController _AngularController;
};

UCLASS()
class VRTOOLKIT_API AAICharacter_Base : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacter_Base();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	void PhysicsAnimationTick(float DeltaTime);

	void EditBoneTransform_NonPhysics(TArray<FTransform>& BoneTransforms);

	void EditBoneTransform_Physics(float DeltaTime, TArray<FTransform>& BoneTransforms);

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* _AnimatedMesh;


	UPROPERTY(EditAnywhere, Category = "Spring")
		FName _RightHandBoneName = "hand_r";

	UPROPERTY(EditAnywhere, Category = "Spring")
		FPIDVectorController _RH_LocPD;

	UPROPERTY(EditAnywhere, Category = "Spring")
		FQuatPDController _RH_RotPD;

	UPROPERTY(EditAnywhere, Category = "Spring")
		FName _RightElbowBoneName = "hand_r";

	UPROPERTY(EditAnywhere, Category = "Spring")
		FPIDVectorController _RE_LocPD;

	UPROPERTY(EditAnywhere, Category = "Spring")
		FQuatPDController _RE_RotPD;

	UPROPERTY(EditAnywhere, Category = "Spring")
		FName _RightShoulderBoneName = "hand_r";

	UPROPERTY(EditAnywhere, Category = "Spring")
		FPIDVectorController _RS_LocPD;

	UPROPERTY(EditAnywhere, Category = "Spring")
		FQuatPDController _RS_RotPD;

	/*UPROPERTY(EditAnywhere, Category = "Animations")
		class UAnimSequence* _TestAnim;

	float _AnimDT = 0;
	float _AnimMaxDT = 0;*/
};
