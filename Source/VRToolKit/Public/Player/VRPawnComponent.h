// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRPawnComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUVRPawnComponent, Log, All);

class UCapsuleComponent;
class USphereComponent;
class UCameraComponent;
class USceneComponent;
class APawn;
class USoundCue;

USTRUCT(BlueprintType)
struct FSmoothJumpParams
{
	GENERATED_BODY()

public:
	bool _bUseSmoothJump = false;

	FVector _JumpStartLocation = FVector::ZeroVector;
	FVector _JumpEndLocation = FVector::ZeroVector;
	FVector _JumpDirection = FVector::ZeroVector;

	float _JumpDistance = 0;
	float _MaxSpeedOfJump = 0;
	float _MinPercentageOfSpeed = 20;

	bool _bKeepJumpSpeed = true;
	bool _bArcJump = false;

};

UENUM(BlueprintType)
enum EMovementModes
{
	EMM_Grounded		UMETA(MetaTag1="Grounded"),
	EMM_Falling			UMETA(MetaTag1 = "Falling&Jumping"),
	EMM_Climbing		UMETA(MetaTag1="Climbing"),
	EMM_Swimming		UMETA(MetaTag1="Swimming"),
	EMM_Flying			UMETA(MetaTag1="Flying"),
	EMM_RampSliding		UMETA(MetaTag1="RampSliding"),
	EMM_Custom			UMETA(MetaTag1="Custom"),
};

UENUM(BlueprintType)
enum EClimbingMode
{
	ECM_None				UMETA(MetaTag1="None"),
	ECM_GrabbedClimbing		UMETA(MetaTag1="GrabbedClimbing"),
	ECM_Vaulting			UMETA(MetaTag1="Vaulting"),
};

USTRUCT()
struct FVRPawnSnapShot
{
	GENERATED_BODY()

	FVector _Location;
	FRotator _Rotation;
};

USTRUCT()
struct FLastFloorHit
{
	GENERATED_BODY()

public:
	bool _bBlockingHit = false;
	bool _WalkableSurface = false;
	FVector _FloorHitNormal = FVector::ZeroVector;
	float _FloorAngle = 0;
	FHitResult _LastFloorHitResult;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTOOLKIT_API UVRPawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVRPawnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddMovementInput(FVector Direction, float Scale = 1);

	void AddRotationInput(FRotator Rot);

	void SetCachedComponents(UCapsuleComponent* Capsule, UCameraComponent* Camera, USceneComponent* VROrigin);

	void FollowCameraPitchRotation(bool bUsePitch);

	void FollowCameraYawRotation(bool bUseYaw);

	void FollowCameraRollRotation(bool bUseRoll);

	void RecenterHMD();

	void RecenterHMDClimbing();

	void HandGrabbedClimbingPoint(bool LeftHand, FQuat ClimbingHandQuat, EClimbingMode AttemptedClimbingMode = EClimbingMode::ECM_GrabbedClimbing);

	void DoJump();

	const FVector GetVelocity();// { return _Velocity; }

private:
	const FVector ConsumeMovementInput();

	const FRotator ConsumeRotationInput();

	void ClientSideHandler(float DeltaTime);

	void SimulatedProxyHandler(float DeltaTime);

	void HandleGroundedMode(float DeltaTime);

	void HandleGroundedModeNew(float DeltaTime);

	bool HandleCharacterStepUp(FHitResult MovementHit, FVector MissingMove, float DeltaTime);

	void HandleFallingMode(float DeltaTime);

	void HandleClimbingMode(float DeltaTime);

	void MoveCapsule(FVector Dir, float MoveAmount, float DeltaTime, bool bIgnoreWalkableSurface = false);

	void MoveCapsule(FVector FinalMove, bool bIgnoreWalkableSurface = false);

	bool IsWalkableSurface(const FHitResult& Hit);

	bool CheckIfWalkableAhead(const FHitResult& Hit);

	void HandleHMDInputRotation(float DeltaTime);

	void DoHMDInputRotation(float Amount);

	void DoFloorCheck(FHitResult& FloorHit);

	FVector WorkOutCapsuleLocationForClimbing();

	FVector WorkOutTwoHandedCapsuleLocationForClimbing();

	FVector WorkOutOneHandedCapsuleLocationForClimbing(bool LeftHand);

	FVector WorkOutGrabbedClimbingModeOffset();

	FVector WorkOutVaultingClimbingModeOffset();

	void ScaleCapsuleToPlayerHeight();

	void HandleJumping(float DeltaTime);

	bool SurfaceSteppable(float DeltaTime, FHitResult& Hit);

	void UpdateXYVelocity(float DeltaTime, FVector DesiredDirection, float DesiredSpeed);

	void ChangeMovementMode(EMovementModes NewMovementMode);

	void HandleRampSliding(float DeltaTime);

	//return true if we can jump
	bool CheckForJump();

	bool LastGroundCheckCausedMovementStateChange();

	void SaveClimbingLocation(float DeltaTime);

	FVector GetClimbingFlingDirection(bool bLeftHand);

	void WorkOutClimbingFlingJumpParams();

	void RotateCapsuleToFaceHeadDirection();

	void RotateItemStorers(float DeltaTime);

	FVector SmoothJumpToLocation(float DeltaTime, FSmoothJumpParams& JumpParams);
private:
	APawn* _OwningPawn = nullptr;

	UCapsuleComponent* _CachedCapsule = nullptr;

	UCameraComponent* _CachedCamera = nullptr;

	USceneComponent* _CachedVROrigin = nullptr;

	USphereComponent* _CachedClimbingDetectionZone = nullptr;

//Variables
private:
	FVector _MovementDirToConsume;
	FRotator _RotationToConsume;

	FVector _LastRecievedLocation;

	bool _bUsingHMD = false;

	/*Current movement state*/
	TEnumAsByte<EMovementModes> _MovementState;

	TEnumAsByte<EClimbingMode> _ClimbingMode;

	FLastFloorHit _LastFloorHit;

	FVector _LastValidFloorNormal = FVector::UpVector;

	FVector _LastClimbingOffset = FVector::ZeroVector;

	//bool _LeftHandGrabbedEGB = false;
	//bool _RightHandGrabbedEGB = false;

	EClimbingMode _LeftHandClimbingMode = EClimbingMode::ECM_None;
	EClimbingMode _RightHandClimbingMode = EClimbingMode::ECM_None;

	bool _InJump = false;
	bool _bIsFlingJump = false;
	bool _CheckFloorStepFallingMode = false;

	FVector _Velocity = FVector::ZeroVector;

	FVector _LastMoveHitNormal = FVector::ZeroVector;

	float _GravityAcceleration = 0;

	TArray<FVector> _CachedClimbingMoves;

	float _ClimbingSaveLocTimer = 0;

	FQuat _LeftHandClimbingQuat = FQuat(1);
	FQuat _RightHandClimbingQuat = FQuat(1);

	FVector _LeftHandClimbingLocation = FVector::ZeroVector;
	FVector _RightHandClimbingLocation = FVector::ZeroVector;

	FSmoothJumpParams _CurrentSmoothJumpParams;
//server functions
private:
	UFUNCTION(Server, Reliable)
		void Server_SendMove(FVector NewLocation);

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_SendMove(FVector NewLocation);

//UPROPERTY Variables
protected:
	/*=================*/
	/*General Settings*/
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: General")
		bool _bUseVRMode = false;

	/*=================*/
	/* Camera Settings */
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: CameraSettings")
		float _MaxCameraDistanceFromCollision = 25.0f;

	/* if true we use smooth turning else we use snap turning */
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: CameraSettings")
		bool _bUseSmoothTurning = true;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: CameraSettings")
		float _SmoothTurningSensitivity = 75.0f;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent: CameraSettings")
		float _SnapTurningAmount = 45.0f;

	/*=================*/
	/*Grounded*/

	//Walking Speed
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		int _WalkSpeed = 250;

	//Running Speed
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		int _RunSpeed = 750;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		float _Acceleration = 25.f;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		float _Decceleration = 25.f;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		float _DirectionChangeSpeed = 25.f;

	//max slope the player can walk up
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		int _MaxWalkableSlope = 55;

	//max stepup height
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		float _MaxStepUpHeight = 25.f;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Grounded")
		float _StepUpVelocity = 25.f;

	/*=================*/
	/*Falling / Jumping*/
	//How Much Gravity should be applied when falling
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Falling/Jumping")
		float _Gravity = 750.0f;

	//How height can we jump
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Falling/Jumping")
		float _JumpZVel = 100;

	/*=================*/
	/*Climbing*/
	//Scale of the character collision box when climbing
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Climbing")
		float _ClimbingCapsuleHalfHeight = 22.f;

	//The gravity strength(1 = full gravity) when we fling ourself from climbing
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Climbing")
		float _ClimbingFlingGravityStrength = .5f;

	//The gravity strength(1 = full gravity) when we fling ourself from climbing
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Climbing")
		float _ClimbingFlingWaitTimeBeforeFalling = .8f;

	//Max Fling Movementspeed
	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Climbing")
		float _ClimbingFlingMovementSpeed = 350.f;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Climbing")
		float _ClimbingFlingMovementDistance = 350.f;

	UPROPERTY(EditAnywhere, Category = "VRPawnComponent MovementMode: Climbing")
		float _MinClimbingMovementSpeedPercentage = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRPawnComponent MovementMode: Audio")
	USoundCue* _SC_Footstep;

private:
	bool _bDoSnapTurning = false;

	FVector _LastLocation = FVector::ZeroVector;

	FVector _VaultingForwardDir = FVector::ZeroVector;

	float _PerchHeight = 5;

	TArray<FVector> _ClimbingFlingAllowedDirections;

	float _CurrentClimbingFlingWaitTime = 0;

	float _CurrentClimbingFlingGravityStrength = 1;

	FVector _FlingJumpGotoLoc = FVector::ZeroVector;

	FVector _ClimbingStartingForwardDir = FVector::ZeroVector;
	
	bool _bUseLeftHandQuat = true;

	FVector _VelocityPerFrame = FVector::ZeroVector;

	float _FootstepTimer = .5f;
};
