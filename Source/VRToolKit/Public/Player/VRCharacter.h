// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VRCharacter.generated.h"

class UVRPawnComponent;
class UCapsuleComponent;
class UCameraComponent;
class USceneComponent;
class UStaticMeshComponent;
class AVRHand;
class AController;
class SkeletalMesh;
class UVRCharacterAB;
class UMotionControllerComponent;
struct FClimbingHandInfo;
enum EClimbingMode;
class USphereComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTOOLKIT_API AVRCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;

	FVector GetCollisionLocation();

	void HandGrabbedClimbingPoint(bool LeftHand, FQuat ClimbingHandQuat, EClimbingMode AttemptedClimbingMode);

	FClimbingHandInfo GetLeftHandClimbInfo();

	FClimbingHandInfo GetRightHandClimbInfo();

	AVRHand* GetLeftHand() { return _LeftHand; }
	AVRHand* GetRightHand() { return _RightHand; }

	const FVector GetDesiredVelocity();

	FVector GetCharacterCollisionLocation();

protected:
	UPROPERTY(EditAnywhere)
	UVRPawnComponent* _VRPawnComp;

	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	void MoveRight(float Value);

	UFUNCTION()
	void YawRotation(float Value);

	UFUNCTION()
	void PitchRotation(float Value);

	UFUNCTION()
	void HostServer();

	UFUNCTION()
	void JoinServer();

	UFUNCTION()
	void SwitchVRMode();

	UFUNCTION()
		void LeftGripPressed(float Value);

	UFUNCTION()
		void RightGripPressed(float Value);

	UFUNCTION()
		void LeftTriggerPressed(float Value);

	UFUNCTION()
		void RightTriggerPressed(float Value);

	UFUNCTION()
		void LeftTopButtonPressed();

	UFUNCTION()
		void LeftBottomButtonPressed();

	UFUNCTION()
		void RightTopButtonPressed();

	UFUNCTION()
		void RightBottomButtonPressed();

	UFUNCTION()
		void DoJump();

	void AddMovementInput(FVector Direction, float Scale = 1);

	void AddRotationInput(FRotator Rotation);

	void SpawnHands();

	void VRBodyTick(float DeltaTime);

	UFUNCTION()
	void RescaleVRBody();
private:
	float _ReplicatedHMDTransformTimer = 0.25f;

	UPROPERTY(EditAnywhere, Category = "ThingsToSpawn")
		TSubclassOf<AVRHand> _BP_DefaultHand;

	AVRHand* _LeftHand = NULL;
	AVRHand* _RightHand = NULL;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* _ClimbingZoneDetection;

	bool _LeftHandGrabbedEGB = false;
	bool _RightHandGrabbedEGB = false;
//Replicated Variables
protected:
	UPROPERTY(Replicated)
		FVector _LastKNownHMDLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
		FRotator _LastKNownHMDRotation = FRotator::ZeroRotator;

	UFUNCTION(BlueprintCallable)
		FTransform GetVRCameraTransorm();

//RPC Functions
protected:
	UFUNCTION(Server, UnReliable)
		void Server_NewHMDTransform(FVector Location, FRotator Rotation);
	
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_SetHands(AVRHand* LeftHand, AVRHand* RightHand);

	UFUNCTION(Server, Reliable)
		void Server_AskForHands();

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_SetNewOwner(AController* NewController);
private:
	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* _CharacterCap;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* _CharacterCam;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* _CharacterRoot;

	UStaticMeshComponent* _HMDMesh;

	UMotionControllerComponent* _LeftHandMCComp;

	UMotionControllerComponent* _RightHandMCComp;

	USceneComponent* _LeftHandRoot;

	USceneComponent* _RightHandRoot;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* _VestMesh;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* _VRBody;

	UPROPERTY(EditAnywhere)
	FName _LeftHandBoneName = "";

	UPROPERTY(EditAnywhere)
	FName _RightHandBoneName = "";

	float _PlayersHeight = 185;

	UVRCharacterAB* _CharacterAB;

	
};
