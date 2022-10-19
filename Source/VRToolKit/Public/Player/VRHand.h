// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utility/PIDControllers.h"
#include "GameFramework/Actor.h"
#include "VRHand.generated.h"


class UMotionControllerComponent;
class UStaticMeshComponent;
class UWidgetInteractionComponent;
class USceneComponent;
class UBoxComponent;
class UItemGrabComponent;
class UPhysicsConstraintComponent;
class AVRItem;
class UPhysicsReplicationComponent;
class UPhysicsHandlerComponent;
class USphereComponent;
class UEnvironmentGrabComponent;
class AVRCharacter;
class UBoxComponent;
class UAnimationAsset;

USTRUCT()
struct FClimbingHandInfo
{
	GENERATED_BODY()

public:
	FVector _GrabbedLocation = FVector::ZeroVector;
	FVector _Hand_CurrentLoc = FVector::ZeroVector;
	FVector _Hand_MoveDiff = FVector::ZeroVector;

	FVector _EGC_StartLocation = FVector::ZeroVector;
	FVector _EGC_CurrentLoc = FVector::ZeroVector;
	FVector _EGC_MoveDiff = FVector::ZeroVector;

	FVector _MC_Location = FVector::ZeroVector;

	FVector _HandToControllerOffset = FVector::ZeroVector;

	FVector _HandForwardDir = FVector::ZeroVector;
};

UCLASS()
class VRTOOLKIT_API AVRHand : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVRHand();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void IsRightHand(bool bRightHand, UMotionControllerComponent* ControllerToFollow);

	bool GetIsRightHand() { return _bRightHand; }

	FVector GetMotionControllerMoveForwardDir();

	//USkeletalMeshComponent* GetPhysicsHandMesh() { return  _ControllerSKM; }
	USkeletalMeshComponent* GetPhysicsHandMesh() { return  _ControllerSKM; }

	void GripPressed();

	void GripReleased();

	void TopButtonPressed(bool bPressed);

	void BottomButtonPressed(bool bPressed);

	void TriggerPressed(float Value);

	const FTransform GetTrackingHandTransform();

	UMotionControllerComponent* GetMotionControllerComponent() { return _MotionControllerComp; }

	void UpdateOwner(AController* PC);

	FTransform GetPhysicsObjectTransform();

	FClimbingHandInfo GetClimbingHandInfo() { return _ClimbingInfo; }

	bool HandInClimbMode() { if (_GrabbedEGC) return true; return false; }

	void SetCharacterAttachedTo(AVRCharacter* Character) { _CharacterAttachedTo = Character; }

	FVector GetDesiredVelocity();

	UPrimitiveComponent* GetHandCollision() { return _ControllerSKM; }

	void NonVRFollow(class USceneComponent* CompToFollow);
protected:
	UPROPERTY(EditAnywhere, Category = "VRHand: Dynamic Edge Grab Detection")
	FVector3f _ParmEdgeDetectBoxExtent = FVector3f(0.5f, 5, 4.f);

	UPROPERTY(EditAnywhere, Category = "VRHand: Dynamic Edge Grab Detection")
	float _MaxSizeCheckUnderParm = 3.f;

	UPROPERTY(EditAnywhere, Category = "VRHand: Dynamic Edge Grab Detection")
	float _MinZSizeOfParmCheck = 3.f;

	UPROPERTY(EditAnywhere, Category = "VRHand: Dynamic Edge Grab Detection")
	FVector3f _FingersEdgeDetectBoxExtent = FVector3f(1.5f, 3, .5f);

	UPROPERTY(EditAnywhere, Category = "VRHand: Dynamic Edge Grab Detection")
		float _MaxTopAngleGrabbable = 45.f;

	UPROPERTY(EditAnywhere, Category = "VRHand: Dynamic Edge Grab Detection")
		float _MinSideAngleGrabbable = 45.f;

	UPROPERTY(EditAnywhere, Category = "VRHand: Dynamic Edge Grab Detection")
		float _MaxFloorAngleWeCanVault = 35.f;

	UPROPERTY(EditAnywhere, Category = "VRHand: Dynamic Edge Grab Detection")
		float _HandHeightClimbingGrabOffset = 15.f;

	UPROPERTY(EditAnywhere, Category = "VRHand: Dynamic Edge Grab Detection")
		float _HandToWallClimbingOffset = 1.f;

	UPROPERTY(EditAnywhere, Category = "VRHand: Vault Detection")
		TArray<FVector> _VaultingLineTracesLocations;

	UPROPERTY(EditAnywhere, Category = "VRHand: Vault Detection")
		float _VaultingLineTraceLength = 2.f;

	UPROPERTY(EditAnywhere, Category = "VRHand: Vault Detection")
		float _MinTraceHitsToAllowVaulting = 0.f;

	UPROPERTY(EditAnywhere, Category = "VRHand: Hand Poses")
		UAnimationAsset* _DefaultHandPose;

	UPROPERTY(EditAnywhere, Category = "VRHand: Hand Posses")
		UAnimationAsset* _EdgeLedgeGrabHandPose;

	UPROPERTY(EditAnywhere, Category = "VRHand: Hand Posses")
		UAnimationAsset* _OtherClimbingGrabHandPose;

	UPROPERTY(EditAnywhere, Category = "VRHand: Hand Posses")
		UAnimationAsset* _VaultingHandPose;

private:
	AVRCharacter* _CharacterAttachedTo;

	UMotionControllerComponent* _MotionControllerComp;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* _ControllerSKM;
	
	UPROPERTY(EditAnywhere, Category = "PhysicsTuning")
	FVector _HandOffset = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FVector _LastKNownHandLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FRotator _LastKNownHandRotation = FRotator::ZeroRotator;

	UPROPERTY(Replicated)
	FVector _LastKNownTrackingHandLocation = FVector::ZeroVector;

	UPROPERTY(Replicated)
	FRotator _LastKNownTrackingHandRotation = FRotator::ZeroRotator;

	float _ReplicatedHandTransformTimer = 0.25f;

	UFUNCTION(Server, UnReliable)
	void Server_NewHandTransform(FVector PLocation, FRotator PRotation, FVector TLocation, FRotator TRotation);

	UFUNCTION(Server, Reliable)
	void Server_GrabbedComponent(UItemGrabComponent* GrabComp);

	UFUNCTION(Server, Reliable)
	void Server_DroppedGrabbedComponent();

	UFUNCTION(Client, Reliable)
	void Client_ItemPickupInvalid();

	void AttemptItemGrab();

	UItemGrabComponent* FindClosestGrabComponent();

	UItemGrabComponent* _ComponentHeld = NULL;

	FCollisionResponseContainer _CorrectResponseContainer;

	bool _bRightHand = false;
	
	void MoveHandOutsideOfHeldObject();

	UPhysicsReplicationComponent* _PRC;

	bool _bDoneGrab = false;

	UPROPERTY(EditAnywhere);
	UPhysicsHandlerComponent* _PHC;

	UPROPERTY(EditAnywhere);
	USphereComponent* _GrabSphere;

	TArray<UItemGrabComponent*> _GrabCompArray;

	TArray<UEnvironmentGrabComponent*> _EnvironmentGrabCompArray;

	UEnvironmentGrabComponent* _GrabbedEGC;

	AActor* _DynamicGrabbed_ActorGrabbed = NULL;

	AActor* _Vaulting_Actor = NULL;

	FClimbingHandInfo _ClimbingInfo;

	void UpdateHandClimbInfo();

	bool CheckDynamicEdgeGrap();

	void CheckForVaulting();

	void ChangeClimbingMode(enum EClimbingMode AttemptedClimbingMode);

	void GripPressCheck();

	UFUNCTION()
		void GrabSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void GrabSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FVector _MC_LastLoc = FVector::ZeroVector;

	float _GripCheckTimer = 0.0f;

	FTransform _ClimbingGrabTransform;
};