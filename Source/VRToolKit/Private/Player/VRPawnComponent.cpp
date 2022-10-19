// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRPawnComponent.h"
#include "..\..\Public\Player\VRPawnComponent.h"
#include "Utility/ExtraMaths.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Player/VRHand.h"
#include "Player/VRCharacter.h"

DEFINE_LOG_CATEGORY(LogUVRPawnComponent);

// Sets default values for this component's properties
UVRPawnComponent::UVRPawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UVRPawnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	{
		//Straight up/down
		_ClimbingFlingAllowedDirections.Add(FVector::UpVector);
		_ClimbingFlingAllowedDirections.Add(-FVector::UpVector);

		//Up Angles
		_ClimbingFlingAllowedDirections.Add(FVector(1, -1, 1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(1, 0, 1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(1, 1, 1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(-1, -1, 1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(-1, 0, 1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(-1, -1, 1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(0, -1, 1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(0, 1, 1).GetSafeNormal());


		//0 Z axis
		_ClimbingFlingAllowedDirections.Add(FVector::RightVector);
		_ClimbingFlingAllowedDirections.Add(-FVector::RightVector);
		_ClimbingFlingAllowedDirections.Add(FVector::ForwardVector);
		_ClimbingFlingAllowedDirections.Add(-FVector::ForwardVector);
		_ClimbingFlingAllowedDirections.Add(FVector(1, -1, 0).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(1, 1, 0).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(-1, 1, 0).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(-1, -1, 0).GetSafeNormal());

		//down Angles
		_ClimbingFlingAllowedDirections.Add(FVector(1, -1, -1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(1, 0, -1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(1, 1, -1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(-1, -1, -1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(-1, 0, -1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(-1, -1, -1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(0, -1, -1).GetSafeNormal());
		_ClimbingFlingAllowedDirections.Add(FVector(0, 1, -1).GetSafeNormal());
	}


	for (int i = 0; i < 10; i++)
	{
		FVector NewLoc = FVector::ZeroVector;
		_CachedClimbingMoves.Add(NewLoc);
	}

	if (APawn* Owner = Cast<APawn>(GetOwner()))
	{
		_OwningPawn = Owner;
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	}

	if (_CachedCapsule)
		_LastRecievedLocation = _CachedCapsule->GetComponentLocation();

	RecenterHMD();
}

// Called every frame
void UVRPawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	ENetRole Role = GetOwnerRole();

	if (_OwningPawn && _OwningPawn->IsLocallyControlled())
	{
		ClientSideHandler(DeltaTime);
		GEngine->AddOnScreenDebugMessage(312, .1f, FColor::Green, "LastFloorHit: " + _LastFloorHit._FloorHitNormal.ToString());
		GEngine->AddOnScreenDebugMessage(313, .1f, FColor::Green, "VelocityDir: " + _Velocity.GetSafeNormal().ToString());
		/*For cases where we havnt consumed the input / rotation vectors, we should do it anyway so it doesnt happen in a different frame
		and be unexpected for the player*/
		/*FVector ConsumedMove = ConsumeMovementInput();
		FRotator ConsumedRot = ConsumeRotationInput();*/
	}
	else if (Role >= ROLE_SimulatedProxy)
	{
		SimulatedProxyHandler(DeltaTime);
	}

	if (_CachedCapsule)
	{
		_LastLocation = _CachedCapsule->GetComponentLocation();
	}
}

void UVRPawnComponent::AddMovementInput(FVector Direction, float Scale)
{
	_MovementDirToConsume += Direction * Scale;
}

void UVRPawnComponent::AddRotationInput(FRotator Rot)
{
	_RotationToConsume += Rot;
}

void UVRPawnComponent::SetCachedComponents(UCapsuleComponent* Capsule, UCameraComponent* Camera, USceneComponent* VROrigin)
{
	_CachedCapsule = Capsule;
	_CachedCamera = Camera;
	_CachedVROrigin = VROrigin;
}

void UVRPawnComponent::FollowCameraPitchRotation(bool bUsePitch)
{

}

void UVRPawnComponent::FollowCameraYawRotation(bool bUseYaw)
{

}

void UVRPawnComponent::FollowCameraRollRotation(bool bUseRoll)
{

}

void UVRPawnComponent::RecenterHMD()
{
	if (!_CachedCamera || !_CachedCapsule || !_CachedVROrigin)
		return;

	FVector SavedCapLoc = _CachedCapsule->GetComponentLocation();

	FVector MoveOffset;
	float HalfHeight = _CachedCapsule->GetScaledCapsuleHalfHeight();

	MoveOffset.X = _CachedCapsule->GetComponentLocation().X - _CachedCamera->GetComponentLocation().X;
	MoveOffset.Y = _CachedCapsule->GetComponentLocation().Y - _CachedCamera->GetComponentLocation().Y;
	MoveOffset.Z = (_CachedCapsule->GetComponentLocation().Z - HalfHeight) - _CachedVROrigin->GetComponentLocation().Z;

	FVector NewLocation = /*_CachedVROrigin->GetComponentLocation() +*/ MoveOffset;
	_CachedVROrigin->AddWorldOffset(MoveOffset);
	_CachedCapsule->SetWorldLocation(SavedCapLoc);
}

void UVRPawnComponent::RecenterHMDClimbing()
{
	if (!_CachedCamera || !_CachedCapsule || !_CachedVROrigin)
		return;

	FVector MoveOffset;
	float HalfHeight = _CachedCapsule->GetScaledCapsuleHalfHeight();

	MoveOffset.X = _CachedCapsule->GetComponentLocation().X - _CachedCamera->GetComponentLocation().X;
	MoveOffset.Y = _CachedCapsule->GetComponentLocation().Y - _CachedCamera->GetComponentLocation().Y;
	MoveOffset.Z = (_CachedCapsule->GetComponentLocation().Z + HalfHeight) - _CachedCamera->GetComponentLocation().Z;

	FVector NewLocation = /*_CachedVROrigin->GetComponentLocation() +*/ MoveOffset;
	_CachedVROrigin->AddWorldOffset(MoveOffset);
	_CachedCapsule->AddWorldOffset(-MoveOffset);
}

void UVRPawnComponent::HandGrabbedClimbingPoint(bool LeftHand, EClimbingMode AttemptedClimbingMode)
{
	if (LeftHand)
		_LeftHandClimbingMode = AttemptedClimbingMode;
	else
		_RightHandClimbingMode = AttemptedClimbingMode;

	if (_LeftHandClimbingMode != EClimbingMode::ECM_None || _RightHandClimbingMode != EClimbingMode::ECM_None)
	{
		_MovementState = EMovementModes::EMM_Climbing;

		if(_LeftHandClimbingMode == EClimbingMode::ECM_GrabbedClimbing || _RightHandClimbingMode == EClimbingMode::ECM_GrabbedClimbing)
			_ClimbingMode = EClimbingMode::ECM_GrabbedClimbing;
		else
		{
			_ClimbingMode = EClimbingMode::ECM_Vaulting;

			if (AVRCharacter* VRC = Cast<AVRCharacter>(_OwningPawn))
			{
				if (LeftHand)
					_VaultingForwardDir = VRC->GetLeftHandClimbInfo()._HandForwardDir;
				else
					_VaultingForwardDir = VRC->GetRightHandClimbInfo()._HandForwardDir;
			}
		}

		if (_CachedCapsule && _CachedCamera)
		{
			_CachedCapsule->SetCapsuleHalfHeight(_ClimbingCapsuleHalfHeight);
			_CachedCapsule->SetWorldLocation(_CachedCamera->GetComponentLocation() 
				- FVector(0, 0, _CachedCapsule->GetScaledCapsuleHalfHeight()));
		}
	}
	else
	{
		_MovementState = EMovementModes::EMM_Falling;
		_ClimbingMode = EClimbingMode::ECM_None;	
		float OldHalfHeight = _CachedCapsule->GetScaledCapsuleHalfHeight();
		float ZMove = _CachedCapsule->GetComponentLocation().Z - _CachedVROrigin->GetComponentLocation().Z;
		ZMove -= OldHalfHeight;
		_CachedCapsule->AddWorldOffset(FVector(0, 0, -ZMove), true);		
		_CachedCapsule->SetCapsuleHalfHeight(92.5f);
		_CachedCapsule->AddWorldOffset(FVector(0, 0, 95.f - OldHalfHeight));
		_CachedCapsule->AddWorldOffset(FVector(0, 0, -5), true);

		_InJump = true;
		_Velocity = GetClimbingFlingDirection() * _ClimbingFlingMovementSpeed;
		_bIsFlingJump = true;
		GEngine->AddOnScreenDebugMessage(526, 50.f, FColor::Blue, _Velocity.ToString());
	}
}

const FVector UVRPawnComponent::ConsumeMovementInput()
{
	_MovementDirToConsume.Normalize();
	FVector ReturnVec = _MovementDirToConsume;
	_MovementDirToConsume = FVector::ZeroVector;
	return ReturnVec;
}

const FRotator UVRPawnComponent::ConsumeRotationInput()
{
	FRotator ReturnRot = _RotationToConsume;
	_RotationToConsume = FRotator::ZeroRotator;
	return ReturnRot;
}

void UVRPawnComponent::ClientSideHandler(float DeltaTime)
{
	if (!_CachedCamera || !_CachedCapsule)
		return;

	_bUsingHMD = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
	

	if (!_bUsingHMD)
	{
		FRotator RotOffset = ConsumeRotationInput();
		_CachedCamera->SetWorldLocation(_CachedCapsule->GetComponentLocation());
		_CachedCamera->AddWorldOffset(FVector(0, 0, _CachedCapsule->GetScaledCapsuleHalfHeight()));

		FRotator CapRot = RotOffset;
		CapRot.Pitch = 0;
		_CachedCapsule->AddLocalRotation(CapRot * _SmoothTurningSensitivity * DeltaTime);

		FRotator OldRot = _CachedCamera->GetComponentRotation();
		_CachedCamera->SetWorldRotation(FRotator::ZeroRotator);
		float YawRot = OldRot.Yaw + (RotOffset.Yaw * _SmoothTurningSensitivity * DeltaTime);
		float PitchRot = OldRot.Pitch + (RotOffset.Pitch * _SmoothTurningSensitivity * DeltaTime);

		if (PitchRot > 80)
		{
			PitchRot = 80;
		}

		if (PitchRot < -80)
		{
			PitchRot = -80;
		}

		_CachedCamera->AddLocalRotation(FRotator(0, YawRot, 0));
		_CachedCamera->AddLocalRotation(FRotator(PitchRot, 0, 0));
	}
	else
	{
		HandleHMDInputRotation(DeltaTime);
	}

	//Handle the correct movement mode
	switch (_MovementState)
	{
	case EMovementModes::EMM_Grounded:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Grounded");
		HandleGroundedModeNew(DeltaTime);
		break;
	case EMovementModes::EMM_Falling:
		GEngine->AddOnScreenDebugMessage(99, .5f, FColor::Yellow, "Movement State: Falling");
		HandleFallingMode(DeltaTime);
		break;
	case EMovementModes::EMM_Climbing:
		HandleClimbingMode(DeltaTime);
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Climbing");
		break;
	case EMovementModes::EMM_Swimming:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Swimming");
		break;
	case EMovementModes::EMM_Flying:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Flying");
		break;
	case EMovementModes::EMM_RampSliding:
		GEngine->AddOnScreenDebugMessage(99, .5f, FColor::Yellow, "Movement State: RampSliding");
		HandleRampSliding(DeltaTime);
		break;
	case EMovementModes::EMM_Custom:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Custom");
		break;
	default:
		break;
	}
	GEngine->AddOnScreenDebugMessage(100, .5f, FColor::Yellow, "MovementSpeed: " + FString::SanitizeFloat(_Velocity.Size()));


	//Send info to server
	FVRPawnSnapShot PSS;
	PSS._Location = _CachedCapsule->GetComponentLocation();
	PSS._Rotation = _CachedCapsule->GetComponentRotation();
	Server_SendMove(PSS._Location);
}

void UVRPawnComponent::SimulatedProxyHandler(float DeltaTime)
{
	if (!_CachedCapsule)
		return;

	//_CachedCapsule->SetWorldLocation(_LastRecievedLocation);
	
	FVector CurrentLoc = _CachedCapsule->GetComponentLocation();
	float InterpSpeed = 15.0f;
	FVector NewLoc;
	NewLoc.X = FMath::FInterpTo(CurrentLoc.X, _LastRecievedLocation.X, DeltaTime, InterpSpeed);
	NewLoc.Y = FMath::FInterpTo(CurrentLoc.Y, _LastRecievedLocation.Y, DeltaTime, InterpSpeed);
	NewLoc.Z = FMath::FInterpTo(CurrentLoc.Z, _LastRecievedLocation.Z, DeltaTime, InterpSpeed);
	_CachedCapsule->SetWorldLocation(NewLoc);

	/*DrawDebugCapsule(GetWorld(), _LastRecievedLocation, _CachedCapsule->GetScaledCapsuleHalfHeight(),
		_CachedCapsule->GetScaledCapsuleRadius(), _CachedCapsule->GetComponentQuat(), FColor::Red, false, 0.01f, 
		(uint8)'\000', 2.5f);*/
}

void UVRPawnComponent::HandleGroundedMode(float DeltaTime)
{
	//ScaleCapsuleToPlayerHeight();

	//Do a floor check
	FHitResult FloorCheckHit;
	bool HitFloor = false;// = DoFloorCheck(FloorCheckHit);

	{
		FVector FloorCheckStart = _CachedCapsule->GetComponentLocation() - FVector(0, 0, _CachedCapsule->GetScaledCapsuleHalfHeight());
		FVector FloorCheckEnd = FloorCheckStart - FVector(0, 0, 1.f);
		FCollisionShape FloorCheckShape = _CachedCapsule->GetCollisionShape();
		FCollisionQueryParams ColParams;
		ColParams.bTraceComplex = false;
		ColParams.AddIgnoredComponent(_CachedCapsule);

		HitFloor = GetWorld()->SweepSingleByChannel(FloorCheckHit, FloorCheckStart, FloorCheckEnd, _CachedCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);
	}

	if (!HitFloor || _InJump)
	{
		bool NotGrounded = false;

		if (!_InJump)
		{
			FHitResult StepDownCheck;
			FVector FloorCheckStart = _CachedCapsule->GetComponentLocation();
			FVector FloorCheckEnd = FloorCheckStart - FVector(0, 0, _MaxStepUpHeight);
			FCollisionShape FloorCheckShape = _CachedCapsule->GetCollisionShape();
			FCollisionQueryParams ColParams;
			ColParams.bTraceComplex = false;
			ColParams.AddIgnoredComponent(_CachedCapsule);

			GetWorld()->SweepSingleByChannel(StepDownCheck, FloorCheckStart, FloorCheckEnd, _CachedCapsule->GetComponentQuat(),
				ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);

			if (StepDownCheck.bBlockingHit)
			{
				_CachedCapsule->AddWorldOffset(FVector(0, 0, -_MaxStepUpHeight), true);
			}
			else
			{
				NotGrounded = true;
			}
		}
		else
		{
			NotGrounded = true;
		}

		if (NotGrounded)
		{
			_MovementState = EMovementModes::EMM_Falling;
			return;
		}
	}

	/* Make capsule face the direction of the movement forward dir (For example the HMD) */
	FRotator Rot = _CachedCapsule->GetComponentRotation();
	Rot.Yaw = _CachedCamera->GetComponentRotation().Yaw;
	_CachedCapsule->SetWorldRotation(Rot);
	
	//DrawDebugLine(GetWorld(), FloorCheckHit.ImpactPoint, FloorCheckHit.ImpactPoint + (FloorCheckHit.ImpactNormal * 50), FColor::Red, false, 5.f);

	//move the play to be ground level
	//_CachedCapsule->SetWorldLocation(FloorCheckHit.Location, true);

	//Check if floor is walkable
	if (IsWalkableSurface(FloorCheckHit) || !FloorCheckHit.bBlockingHit)
	{
		FVector MoveDir = ConsumeMovementInput();

		if (MoveDir != FVector::ZeroVector)
		{
			//MoveDir = FVector::VectorPlaneProject(MoveDir, FloorCheckHit.Normal);
			//MoveDir = FVector::VectorPlaneProject(MoveDir, _LastFloorHit._FloorHitNormal);
			//MoveCapsule(MoveDir, _RunSpeed, DeltaTime, true);
			_Velocity = MoveDir * _RunSpeed;
			
			if (_bUsingHMD)
			{
				RecenterHMD();	
			}
		}
		else
		{
			_Velocity.X = 0;
			_Velocity.Y = 0;
		}
	}
	/*else
	{
		GEngine->AddOnScreenDebugMessage(111, 1.f, FColor::Yellow, "Movement State: Grounded: Sliding");
		FVector MoveDir = FloorCheckHit.Normal;
		//FVector MoveDir = _LastFloorHit._FloorHitNormal;
		MoveDir.Z = 0;
		MoveDir.Normalize();
		MoveDir = FVector::VectorPlaneProject(MoveDir, FloorCheckHit.Normal);
		//MoveDir = FVector::VectorPlaneProject(MoveDir, _LastFloorHit._FloorHitNormal);

		float DotP = FVector::DotProduct(FVector(0, 0, 1), FloorCheckHit.Normal);
		//float DotP = FVector::DotProduct(FVector(0, 0, 1), _LastFloorHit._FloorHitNormal);
		float SlideSpeed = (_RunSpeed * (2 - DotP)) / 2;
		GEngine->AddOnScreenDebugMessage(98, .1f, FColor::Yellow, "Slide Speed: " + FString::SanitizeFloat(SlideSpeed));
		//DrawDebugLine(GetWorld(), FloorCheckHit.ImpactPoint, FloorCheckHit.ImpactPoint + (MoveDir * 50), FColor::Green, false, 5.f);

		//MoveCapsule(MoveDir, SlideSpeed, DeltaTime, true);
		//_Velocity = MoveDir * SlideSpeed;
		if (_bUsingHMD)
		{
			RecenterHMD();
		}
	}*/

	FVector Dir = _Velocity;
	Dir.Normalize();
	float Amount = _Velocity.Size();

	MoveCapsule(Dir, Amount, DeltaTime, false);
}

void UVRPawnComponent::HandleGroundedModeNew(float DeltaTime)
{
	if(!_CachedCapsule)
		return;

	if (CheckForJump())
		return;

	//Do a floor check
	FHitResult FloorHit;
	DoFloorCheck(FloorHit);
	
	/*FHitResult FloorCheckHit;
	bool HitFloor = false;// = DoFloorCheck(FloorCheckHit);*/

	{
		/*FVector FloorCheckStart = _CachedCapsule->GetComponentLocation();
		FVector FloorCheckEnd = FloorCheckStart - FVector(0, 0, _PerchHeight + 1);
		FCollisionShape FloorCheckShape = _CachedCapsule->GetCollisionShape();
		FCollisionQueryParams ColParams;
		ColParams.bTraceComplex = false;
		ColParams.AddIgnoredComponent(_CachedCapsule);

		HitFloor = GetWorld()->SweepSingleByChannel(FloorCheckHit, FloorCheckStart, FloorCheckEnd, _CachedCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);

		if (!_LastFloorHit._bBlockingHit || !_LastFloorHit._WalkableSurface)
		{
			_MovementState = EMovementModes::EMM_Falling;
			return;
		}*/

		if (LastGroundCheckCausedMovementStateChange())
			return;
	}

	/* Make capsule face the direction of the movement forward dir (For example the HMD) */
	FRotator Rot = _CachedCapsule->GetComponentRotation();
	Rot.Yaw = _CachedCamera->GetComponentRotation().Yaw;
	_CachedCapsule->SetWorldRotation(Rot);

	FVector Input = ConsumeMovementInput();
	FVector MoveDir = Input.GetSafeNormal();


	float FloorDotP = _LastFloorHit._FloorHitNormal | MoveDir;
	FVector NewMoveDir = MoveDir;
	NewMoveDir.Z = -FloorDotP / _LastFloorHit._FloorHitNormal.Z;

	if(Input != FVector::ZeroVector)
		UpdateXYVelocity(DeltaTime, NewMoveDir.GetSafeNormal(), _RunSpeed);
	else
		UpdateXYVelocity(DeltaTime, _Velocity.GetSafeNormal(), 0);


	//bool bSteppingUp = HandleCharacterStepUp(DeltaTime);
	FVector Dir = _Velocity.GetSafeNormal();
	float Amount = _Velocity.Size();
	MoveCapsule(Dir, Amount, DeltaTime);
	
	//_CachedCapsule->AddWorldOffset(FVector(0, 0, -_Gravity * DeltaTime), true);

	if (_bUsingHMD && _Velocity.Size() > KINDA_SMALL_NUMBER)
		RecenterHMD();

	//Move the character off the ground a bit so we dont penetrate into it
	/*{
		FHitResult HeightCheck;
		FVector HeightCheckStart = _CachedCapsule->GetComponentLocation();
		float ScaledHalfHeight = _CachedCapsule->GetScaledCapsuleHalfHeight();
		float ScaledRadius = _CachedCapsule->GetScaledCapsuleRadius();

		float ZHeightOffset = ScaledHalfHeight - ScaledRadius + 12.f;
		FVector HeightCheckEnd = HeightCheckStart - FVector(0, 0, ZHeightOffset);

		FCollisionShape FloorCheckShape;
		FloorCheckShape.SetSphere(ScaledRadius);

		FCollisionQueryParams ColParams;
		ColParams.bTraceComplex = false;
		ColParams.AddIgnoredComponent(_CachedCapsule);

		GetWorld()->SweepSingleByChannel(HeightCheck, HeightCheckStart, HeightCheckEnd, _CachedCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);

		//if (HeightCheck.bBlockingHit)
			//_CachedCapsule->SetWorldLocation(HeightCheck.Location + FVector(0, 0, ScaledHalfHeight - ScaledRadius + 5));
	}*/

	
	//GEngine->AddOnScreenDebugMessage(314, .1f, FColor::Green, "LastFloorHit: " + _LastValidFloorNormal.ToString());

}

bool UVRPawnComponent::HandleCharacterStepUp(FHitResult MovementHit, FVector MissingMove, float DeltaTime)
{
	if (!_CachedCapsule)
		return false;

	FVector CapsuleLoc = _CachedCapsule->GetComponentLocation();
	float CapRadius = _CachedCapsule->GetScaledCapsuleRadius();

	FHitResult StepUpCheck;
	FVector StepUpCheckStart = /*CapsuleLoc + MissingMoveMovementHit.TraceEnd;*/ MovementHit.ImpactPoint;
	StepUpCheckStart.Z = CapsuleLoc.Z + _MaxStepUpHeight;
	FVector StepUpCheckEnd = StepUpCheckStart - FVector(0, 0, _MaxStepUpHeight + SMALL_NUMBER);
	FCollisionShape FloorCheckShape = _CachedCapsule->GetCollisionShape();
	FCollisionQueryParams ColParams;
	ColParams.bTraceComplex = false;
	ColParams.AddIgnoredComponent(_CachedCapsule);

	GetWorld()->SweepSingleByChannel(StepUpCheck, StepUpCheckStart, StepUpCheckEnd, _CachedCapsule->GetComponentQuat(),
		ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);

	if (StepUpCheck.IsValidBlockingHit() && StepUpCheck.Location.Z > CapsuleLoc.Z)
	{
		if (IsWalkableSurface(StepUpCheck))
		{
			float HeightMove = ((StepUpCheck.Location.Z - CapsuleLoc.Z));

			if(_Velocity.Z < 0)
				HeightMove += -_Velocity.Z * DeltaTime;

			_CachedCapsule->AddWorldOffset(FVector(0, 0, HeightMove + _PerchHeight), true);

			//if we are falling then the missing move will have a z value which we dont want to use
			_CachedCapsule->AddWorldOffset(FVector(MissingMove.X, MissingMove.Y, 0), true);
			return true;
		}
	}

	return false;
}

void UVRPawnComponent::HandleFallingMode(float DeltaTime)
{
	if (!_CachedCapsule)
		return;

	if(!_bIsFlingJump)
		_Velocity.Z -= _Gravity * DeltaTime;
	else
		_Velocity.Z -= (_Gravity * _ClimbingFlingGravityStrength) * DeltaTime;

	if(_Velocity.Z <= 0)
		_Velocity.Z = FMath::Clamp(_Velocity.Z, -_Gravity, 0);

	if (_Velocity.Z <= 0 && _InJump)
		_InJump = false;
	
	if (_bUsingHMD)
		RecenterHMD();
	//ScaleCapsuleToPlayerHeight();

	if (!_InJump)
	{
		_bIsFlingJump = false;
		FHitResult FloorHit;
		DoFloorCheck(FloorHit);
		/*bool FloorCheck = false;*/// = DoFloorCheck(FloorHit);

		/*FVector FloorCheckStart = _CachedCapsule->GetComponentLocation();
		FVector FloorCheckEnd = FloorCheckStart - FVector(0, 0, 1.5f);
		FCollisionShape FloorCheckShape = _CachedCapsule->GetCollisionShape();
		FCollisionQueryParams ColParams;
		ColParams.bTraceComplex = false;
		ColParams.AddIgnoredComponent(_CachedCapsule);

		FloorCheck = GetWorld()->SweepSingleByChannel(FloorHit, FloorCheckStart, FloorCheckEnd, _CachedCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);

		if (_LastFloorHit._bBlockingHit && _LastFloorHit._WalkableSurface)
		{
			_MovementState = EMovementModes::EMM_Grounded;
			_Velocity.Z = 0;
			return;
		}*/

		if (LastGroundCheckCausedMovementStateChange())
			return;
	}

	//check to see if we hit something above us (like hitting your head on a something above) and if we do then the jump needs to stop
	if(_Velocity.Z > 0)
	{
		FHitResult RoofHit;
		
		FVector RoofHitStart = _CachedCapsule->GetComponentLocation();
		FVector RoofHitEnd = RoofHitStart + FVector(0, 0, 1.5f);
		FCollisionShape FloorCheckShape = _CachedCapsule->GetCollisionShape();
		FCollisionQueryParams ColParams;
		ColParams.bTraceComplex = false;
		ColParams.AddIgnoredComponent(_CachedCapsule);

		GetWorld()->SweepSingleByChannel(RoofHit, RoofHitStart, RoofHitEnd, _CachedCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);
		
		if (RoofHit.IsValidBlockingHit())
		{
			_Velocity.Z = 0;
		}
	}

	//If we jump into a corner we get stuck so we need to push the player away from it
	if (_Velocity.X != 0 || _Velocity.Y != 0)
	{
		FVector XYVel = FVector(_Velocity.X, _Velocity.Y, 0);
		FHitResult WallSlideCheck;

		FVector WallSlideCheckStart = _CachedCapsule->GetComponentLocation();
		FVector WallSlideCheckEnd = WallSlideCheckStart + (_Velocity.GetSafeNormal() * 1);
		FCollisionShape FloorCheckShape = _CachedCapsule->GetCollisionShape();
		FCollisionQueryParams ColParams;
		ColParams.bTraceComplex = false;
		ColParams.AddIgnoredComponent(_CachedCapsule);

		GetWorld()->SweepSingleByChannel(WallSlideCheck, WallSlideCheckStart, WallSlideCheckEnd, _CachedCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);

		if (WallSlideCheck.bBlockingHit)
		{
			XYVel = FVector::VectorPlaneProject(XYVel.GetSafeNormal(), WallSlideCheck.ImpactNormal);
			_Velocity.X = XYVel.X;
			_Velocity.Y = XYVel.Y;
		}
	}

	FVector Dir = _Velocity.GetSafeNormal();
	float Amount = _Velocity.Size();
	MoveCapsule(Dir, Amount, DeltaTime, false);
}

void UVRPawnComponent::HandleClimbingMode(float DeltaTime)
{
	if (!_CachedCapsule || !_CachedVROrigin || !_CachedCamera)
		return;

	FVector Offset = FVector::ZeroVector;

	switch (_ClimbingMode)
	{
	case EClimbingMode::ECM_GrabbedClimbing:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Grabbed Climbing");
		Offset = WorkOutGrabbedClimbingModeOffset();
		break;
	case EClimbingMode::ECM_Vaulting:
		GEngine->AddOnScreenDebugMessage(99, .1f, FColor::Yellow, "Movement State: Vaulting Climbing");
		Offset = WorkOutVaultingClimbingModeOffset();
		break;
	default:
		break;
	}
	
	UpdateXYVelocity(DeltaTime, _Velocity.GetSafeNormal(), 0);

	//Offset = WorkOutGrabbedClimbingModeOffset();
	//Cant use the 'MoveCapsule' function because when when we have a blocking hit, it uses the missing move to slide along the surface,
	//we dont want this for climbing because it makes the player slide around the wall that we dont want, instead we use trig to calculate
	//where about we should be against the wall
	FHitResult Hit;
	_CachedCapsule->AddWorldOffset(Offset, true, &Hit);

	if (Hit.bBlockingHit)
	{
		FVector CapLoc = _CachedCapsule->GetComponentLocation();
		FVector Dir = (CapLoc - Hit.TraceEnd).GetSafeNormal();
		float H = FVector::Distance(CapLoc, Hit.TraceEnd);
		float P = ExtraMaths::GetAngleOfTwoVectors(Dir, Hit.ImpactNormal);
		float A = H * FMath::Cos(FMath::DegreesToRadians(P));
		FVector NewLoc = Hit.TraceEnd + (Hit.ImpactNormal * A);
		FVector NewOffset = NewLoc - _CachedCapsule->GetComponentLocation();
		_CachedCapsule->AddWorldOffset(NewOffset, true);
	}

	RecenterHMDClimbing();

	SaveClimbingLocation(DeltaTime);
}

void UVRPawnComponent::MoveCapsule(FVector Dir, float MoveAmount, float DeltaTime, bool bIgnoreWalkableSurface)
{
	if (!_CachedCapsule || MoveAmount == 0)
		return;

	FVector LocationBeforeMove = _CachedCapsule->GetComponentLocation();

	/* Move the character capsule in a direction */
	FHitResult MovementHit;
	FVector Offset = (Dir * MoveAmount) * DeltaTime;
	_CachedCapsule->AddWorldOffset(Offset, true, &MovementHit);

	bool bHitUnWalkableSurface = false;


	if (MovementHit.IsValidBlockingHit())
	{	
		if (bIgnoreWalkableSurface || IsWalkableSurface(MovementHit))
		{
			FVector PlaneProject = FVector::VectorPlaneProject(Offset, MovementHit.ImpactNormal);
			//PlaneProject.Normalize();
			Offset = PlaneProject * (1 - MovementHit.Time);
			_CachedCapsule->AddWorldOffset(Offset, true, &MovementHit);
			//_Velocity = PlaneProject.GetSafeNormal() * MoveAmount;	

			if (bIgnoreWalkableSurface || IsWalkableSurface(MovementHit))
			{
				PlaneProject = FVector::VectorPlaneProject(Offset, MovementHit.ImpactNormal);
				//PlaneProject.Normalize();
				Offset = PlaneProject * (1 - MovementHit.Time);
				_CachedCapsule->AddWorldOffset(Offset, true, &MovementHit);
			}
			else
			{
				bHitUnWalkableSurface = true;
			}
		}
		else 
		{				
			bHitUnWalkableSurface = true;
		}
	}

	if (bHitUnWalkableSurface)
	{
		FVector CapsuleLoc = _CachedCapsule->GetComponentLocation();
		FVector MissingMove = Offset * (1 - MovementHit.Time);

		if (_MovementState != EMovementModes::EMM_Grounded || !HandleCharacterStepUp(MovementHit, MissingMove, DeltaTime))
		{
			FVector Normal = MovementHit.ImpactNormal;
			Normal.Z = 0;
			Normal.Normalize();

			//ExtraMaths::CorrectNormalizedVector(Normal);
			FVector PlaneProject = FVector::VectorPlaneProject(Offset, Normal);
			Offset = PlaneProject * (1 - MovementHit.Time);
			_CachedCapsule->AddWorldOffset(Offset, true);
			//_Velocity = PlaneProject.GetSafeNormal() * MoveAmount;
		}
	}
}

void UVRPawnComponent::MoveCapsule(FVector FinalMove, bool bIgnoreWalkableSurface)
{
	if (!_CachedCapsule)
		return;

	/* Move the character capsule in a direction */
	FHitResult MovementHit;
	_CachedCapsule->AddWorldOffset(FinalMove, true, &MovementHit);

	if (MovementHit.bBlockingHit)
	{
		if (bIgnoreWalkableSurface || IsWalkableSurface(MovementHit))
		{
			FVector PlaneProject = FVector::VectorPlaneProject(FinalMove, MovementHit.ImpactNormal);
			//ExtraMaths::CorrectNormalizedVector(PlaneProject);
			//PlaneProject.Normalize();
			FinalMove = PlaneProject * (1 - MovementHit.Time);
			_CachedCapsule->AddWorldOffset(FinalMove, true);
		}
		else
		{
			FVector Normal = MovementHit.ImpactNormal;
			Normal.Z = 0;
			Normal.Normalize();
			//ExtraMaths::CorrectNormalizedVector(Normal);

			FVector PlaneProject = FVector::VectorPlaneProject(FinalMove, Normal);
			FinalMove = PlaneProject * (1 - MovementHit.Time);
			_CachedCapsule->AddWorldOffset(FinalMove, true);
		}
	}

}

bool UVRPawnComponent::IsWalkableSurface(const FHitResult& Hit)
{
	if (!_CachedCapsule)
		return false;

	if (!Hit.bBlockingHit || Hit.ImpactNormal.Z < KINDA_SMALL_NUMBER)
		return false;

	//DrawDebugLine(GetWorld(), Hit.Location, Hit.Location + (Hit.ImpactNormal * 50), FColor::Blue, false, 0.1f);
	//DrawDebugLine(GetWorld(), Hit.Location, Hit.Location + (Hit.Normal * 50), FColor::Red, false, 0.1f);
	float Angle = ExtraMaths::GetAngleOfTwoVectors(FVector(0, 0, 1), Hit.ImpactNormal);

	if (Angle > _MaxWalkableSlope)
		return false;

	return true;
}

bool UVRPawnComponent::CheckIfWalkableAhead(const FHitResult& Hit)
{
	if (!_CachedCapsule)
		return false;

	if (!Hit.bBlockingHit || Hit.ImpactNormal.Z < KINDA_SMALL_NUMBER)
		return false;

	//DrawDebugLine(GetWorld(), Hit.Location, Hit.Location + (Hit.ImpactNormal * 50), FColor::Blue, false, 0.1f);
	//DrawDebugLine(GetWorld(), Hit.Location, Hit.Location + (Hit.Normal * 50), FColor::Red, false, 0.1f);
	float Angle = ExtraMaths::GetAngleOfTwoVectors(FVector(0, 0, 1), Hit.ImpactNormal);

	if (Angle > _MaxWalkableSlope)
	{
		/*
		* Sometimes the edges of collisions(for example round corners) can detect it as unwalkable event though in reality we should
		* to fix this we should check a bit ahead and see if that is walkable
		*/
		FVector CapsuleLoc = _CachedCapsule->GetComponentLocation();

		FHitResult AHeadHit;
		FVector VelocityDir = _Velocity.GetSafeNormal();
		VelocityDir.Z = 0;
		FVector AHeadCheckStart = CapsuleLoc + (VelocityDir.GetSafeNormal() * _CachedCapsule->GetScaledCapsuleRadius());//MovementHit.ImpactPoint;
		AHeadCheckStart.Z += _MaxStepUpHeight;
		FVector AHeadCheckEnd = AHeadCheckStart - FVector(0, 0, _MaxStepUpHeight + 5);
		FCollisionShape FloorCheckShape = _CachedCapsule->GetCollisionShape();
		FCollisionQueryParams ColParams;
		ColParams.bTraceComplex = false;
		ColParams.AddIgnoredComponent(_CachedCapsule);

		GetWorld()->SweepSingleByChannel(AHeadHit, AHeadCheckStart, AHeadCheckEnd, _CachedCapsule->GetComponentQuat(),
			ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);

		DrawDebugCapsule(GetWorld(), AHeadHit.Location, FloorCheckShape.GetCapsuleHalfHeight(), FloorCheckShape.GetCapsuleRadius(),
			_CachedCapsule->GetComponentQuat(), FColor::Green, false, 5.f);

		float AHeadAngle = ExtraMaths::GetAngleOfTwoVectors(FVector(0, 0, 1), AHeadHit.ImpactNormal);

		if (AHeadHit.bBlockingHit && AHeadAngle < _MaxWalkableSlope)
		{
			GEngine->AddOnScreenDebugMessage(656, .1f, FColor::Yellow, "A Head Angle: " + FString::SanitizeFloat(Angle) + ": IsWalkable", false);
			return true;
		}

		return false;
	}

	return true;
}

void UVRPawnComponent::HandleHMDInputRotation(float DeltaTime)
{
	FRotator RotOffset = ConsumeRotationInput();

	if (!_bDoSnapTurning && (RotOffset.Yaw > -0.3f && RotOffset.Yaw < 0.3f))
		_bDoSnapTurning = true;

	float FinalAmount = 0;
	_bUseSmoothTurning ? FinalAmount = _SmoothTurningSensitivity * RotOffset.Yaw * DeltaTime
		: FinalAmount = _SnapTurningAmount * (RotOffset.Yaw > 0 ? 1 : -1);

	if (_bUseSmoothTurning || (_bDoSnapTurning && (RotOffset.Yaw > 0.5f || RotOffset.Yaw < -0.5f)))
	{
		DoHMDInputRotation(FinalAmount);
		_bDoSnapTurning = false;
	}
}

void UVRPawnComponent::DoHMDInputRotation(float Amount)
{
	if (!_CachedVROrigin || !_CachedCamera || !_CachedCapsule)
		return;

	FVector OldCapLoc = _CachedCapsule->GetComponentLocation();
	//New Rotation
	FVector Distance = _CachedVROrigin->GetComponentLocation() - _CachedCamera->GetComponentLocation();
	FVector Rotation = Distance.RotateAngleAxis(Amount, FVector(0, 0, 1));
	FVector FinalLocation = _CachedCamera->GetComponentLocation() + Rotation;

	_CachedVROrigin->SetWorldLocation(FinalLocation);
	_CachedVROrigin->AddRelativeRotation(FRotator(0, Amount, 0));

	//Need to move the cap back to where it was otherwhise it gets stuck in geometry
	_CachedCapsule->SetWorldLocation(OldCapLoc);
	
}

void UVRPawnComponent::DoFloorCheck(FHitResult& FloorHit)
{
	FLastFloorHit OldHit;
	float CapHalfHeight = _CachedCapsule->GetScaledCapsuleHalfHeight();
	float CapRadius = _CachedCapsule->GetScaledCapsuleRadius();
	float SweepHalfHeight = CapHalfHeight * 0.8f;
	float SweepRadius = CapRadius * 0.8f;
	FVector CapLoc = _CachedCapsule->GetComponentLocation();

	FVector FloorCheckStart = CapLoc - FVector(0, 0, CapHalfHeight - SweepHalfHeight);
	FVector FloorCheckEnd = FloorCheckStart - FVector(0, 0, _PerchHeight + _MaxStepUpHeight);
	FCollisionShape FloorCheckShape = FCollisionShape::MakeCapsule(SweepRadius, SweepHalfHeight);
	FCollisionQueryParams ColParams;
	ColParams.bTraceComplex = false;
	ColParams.AddIgnoredComponent(_CachedCapsule);

	GetWorld()->SweepSingleByChannel(FloorHit, FloorCheckStart, FloorCheckEnd, _CachedCapsule->GetComponentQuat(),
		ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);

	bool bIngnorePerchHit = false;
	if (FloorHit.IsValidBlockingHit())
	{		
		_LastFloorHit._bBlockingHit = true;
		_LastFloorHit._LastFloorHitResult = FloorHit;
		_LastFloorHit._FloorHitNormal = FloorHit.ImpactNormal;
		if (IsWalkableSurface(FloorHit))
		{
			_LastFloorHit._WalkableSurface = true;			
		}
		else /*if(_MovementState == EMovementModes::EMM_Grounded)*/
		{
			//if we are on an unwalkable surface, then line trace down to check if we are on an walkable surface
			FHitResult LineFloorHit;
			GetWorld()->LineTraceSingleByChannel(LineFloorHit, FloorCheckStart + FVector(0, 0, 2), FloorCheckEnd - FVector(0, 0, CapHalfHeight)
				, ECollisionChannel::ECC_Pawn, ColParams);

			bIngnorePerchHit = true;

			if (IsWalkableSurface(LineFloorHit))
			{
				//if our trace hit something then we override our floor hit
				_LastFloorHit._WalkableSurface = true;
				_LastFloorHit._bBlockingHit = true;
				_LastFloorHit._LastFloorHitResult = LineFloorHit;
				_LastFloorHit._FloorHitNormal = LineFloorHit.ImpactNormal;	
			}
			else
				_LastFloorHit._WalkableSurface = false;
		}
	}
	else
	{
		_LastFloorHit._bBlockingHit = false;
		_LastFloorHit._LastFloorHitResult = FloorHit;
		_LastFloorHit._FloorHitNormal = FloorHit.ImpactNormal;
		_LastFloorHit._WalkableSurface = false;
		bIngnorePerchHit = true;
	}

	//Move cap above the ground
	float LastZ = _CachedCapsule->GetComponentLocation().Z;
	if (!bIngnorePerchHit && _LastFloorHit._bBlockingHit /*&& _LastFloorHit._WalkableSurface*/)
	{
		float DistDFromFloor = (CapLoc.Z - CapHalfHeight) - (FloorHit.Location.Z - SweepHalfHeight);
		float PerchOffset = _PerchHeight - DistDFromFloor;
		PerchOffset = FMath::Clamp(PerchOffset, -_PerchHeight, _PerchHeight);
		_CachedCapsule->AddWorldOffset(FVector(0, 0, PerchOffset), true);

		if (FMath::Abs(LastZ - _CachedCapsule->GetComponentLocation().Z) > 1)
		{
			RecenterHMD();
			RecenterHMD();
		}
	}
}

void UVRPawnComponent::DoJump()
{
	/*if (_InJump || !_CachedCapsule || _MovementState == EMovementModes::EMM_Falling)
		return;

	_Velocity.Z = _JumpZVel;*/
	_InJump = true;
}

const FVector UVRPawnComponent::GetVelocity()
{
	FVector VelocityFrame = _Velocity;

	if (_MovementState == EMovementModes::EMM_Climbing)
		VelocityFrame = FVector::ZeroVector;

	return VelocityFrame;
}

FVector UVRPawnComponent::WorkOutCapsuleLocationForClimbing()
{
	FVector NewCapLoc = FVector::ZeroVector;

	bool bGetLeftHandOffset = false;
	bool bGetRightHandOffset = false;

	if (_LeftHandClimbingMode != EClimbingMode::ECM_None)
	{
		bGetLeftHandOffset = true;
	}
	
	if (_RightHandClimbingMode != EClimbingMode::ECM_None)
	{
		bGetRightHandOffset = true;
	}

	if (bGetLeftHandOffset && bGetRightHandOffset)
		NewCapLoc = WorkOutTwoHandedCapsuleLocationForClimbing();
	else if (bGetLeftHandOffset)
		NewCapLoc = WorkOutOneHandedCapsuleLocationForClimbing(true);
	else if (bGetRightHandOffset)
		NewCapLoc = WorkOutOneHandedCapsuleLocationForClimbing(false);

	return NewCapLoc;
}

FVector UVRPawnComponent::WorkOutTwoHandedCapsuleLocationForClimbing()
{
	if (!_OwningPawn)
		return FVector::ZeroVector;

	FClimbingHandInfo LeftHandClimbInfo;
	FClimbingHandInfo RightHandClimbInfo;

	if (AVRCharacter* VRC = Cast<AVRCharacter>(_OwningPawn))
	{
		LeftHandClimbInfo = VRC->GetLeftHandClimbInfo();
		RightHandClimbInfo = VRC->GetRightHandClimbInfo();

		FVector FinalLefttLoc = LeftHandClimbInfo._GrabbedLocation + LeftHandClimbInfo._EGC_MoveDiff;
		FVector FinalRightLoc = RightHandClimbInfo._GrabbedLocation + RightHandClimbInfo._EGC_MoveDiff;
		

		//Find Center Point of grabbed location
		FVector GrabbedCentrePoint = FinalRightLoc - FinalLefttLoc;
		GrabbedCentrePoint /= 2;
		GrabbedCentrePoint += FinalLefttLoc;

		//find center point of current hand locations
		FVector CurrentCentrePoint = RightHandClimbInfo._MC_Location - LeftHandClimbInfo._MC_Location;
		CurrentCentrePoint /= 2;
		CurrentCentrePoint += LeftHandClimbInfo._MC_Location;

		// find difference for the offset
		FVector Offset = CurrentCentrePoint - GrabbedCentrePoint;
		return -Offset;
	}

	return FVector::ZeroVector;
}

FVector UVRPawnComponent::WorkOutOneHandedCapsuleLocationForClimbing(bool LeftHand)
{
	FClimbingHandInfo HandClimbInfo;

	if (AVRCharacter* VRC = Cast<AVRCharacter>(_OwningPawn))
	{
		if (LeftHand)
			HandClimbInfo = VRC->GetLeftHandClimbInfo();
		else
			HandClimbInfo = VRC->GetRightHandClimbInfo();

		return -HandClimbInfo._Hand_MoveDiff;
	}

	return FVector::ZeroVector;
}

FVector UVRPawnComponent::WorkOutGrabbedClimbingModeOffset()
{
	return WorkOutCapsuleLocationForClimbing();
}

FVector UVRPawnComponent::WorkOutVaultingClimbingModeOffset()
{
	if (!_CachedCapsule)
		return FVector::ZeroVector;

	FVector CapLoc = _CachedCapsule->GetComponentLocation();
	FVector HandOffset = WorkOutCapsuleLocationForClimbing();
	FVector CapLocPlusOffset = CapLoc + HandOffset;
	FVector ForwardDir = _CachedCapsule->GetForwardVector();
	
	FVector NewLocOnLine = ExtraMaths::PointProjectionOnLine(CapLoc, CapLoc + (ForwardDir * 1000), CapLoc + HandOffset, false);
	FVector FinalOffset = NewLocOnLine - CapLoc;
	FinalOffset.Z = HandOffset.Z;
	return FinalOffset;
}

void UVRPawnComponent::ScaleCapsuleToPlayerHeight()
{
	return;

	if (!_CachedVROrigin || !_CachedCamera || !_CachedCapsule)
		return;

	float NewHalfHeight = _CachedCamera->GetComponentLocation().Z - _CachedVROrigin->GetComponentLocation().Z;
	NewHalfHeight /= 2;

	if (NewHalfHeight < _CachedCapsule->GetScaledCapsuleHalfHeight())
	{
		_CachedCapsule->SetCapsuleHalfHeight(NewHalfHeight);
		_CachedCapsule->AddWorldOffset(FVector(0, 0, -NewHalfHeight), true);
	}
	else
	{
		_CachedCapsule->SetCapsuleHalfHeight(NewHalfHeight);
		_CachedCapsule->AddWorldOffset(FVector(0, 0, NewHalfHeight * 1.5f));
		_CachedCapsule->AddWorldOffset(FVector(0, 0, -NewHalfHeight * 2), true);
	}
}

void UVRPawnComponent::HandleJumping(float DeltaTime)
{
	/*if (!_InJump || !_CachedCapsule || _CachedCapsule->GetComponentLocation().Z > _JumpEndZ)
		return;

	float JumpOffset = 0;

	if (_CachedCapsule->GetComponentLocation().Z + _JumpAcceleration > _JumpEndZ)
	{		
		_InJump = false;
		_Velocity.Z = _JumpAcceleration;
	}
	else
	{
		JumpOffset = _JumpAcceleration;
	}

	FHitResult Hit;
	FVector Offset = FVector(_LastMoveOffset.X, _LastMoveOffset.Y, JumpOffset);
	FVector Dir = Offset;
	Dir.Normalize();
	float Amount = Offset.Size();
	//MoveCapsule(Dir, Amount, DeltaTime, true);
	_Velocity.Z += _JumpAcceleration;
	GEngine->AddOnScreenDebugMessage(53, 5.0f, FColor::Red, "Jumped");

	if (Hit.bBlockingHit)
	{
		_InJump = false;
		_Velocity.Z = 0;
	}*/
}

bool UVRPawnComponent::SurfaceSteppable(float DeltaTime, FHitResult& Hit)
{
	if (!_CachedCapsule)
		return false;

	FCollisionShape Shape = _CachedCapsule->GetCollisionShape();

	FVector GoingDir = _Velocity.GetSafeNormal() * _CachedCapsule->GetScaledCapsuleRadius() * 2;
	GoingDir.Z += _MaxStepUpHeight;
	FVector StartLoc = _CachedCapsule->GetComponentLocation() + GoingDir;
	FVector EndLoc = StartLoc - FVector(0, 0, _MaxStepUpHeight * 2);
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	Params.AddIgnoredComponent(_CachedCapsule);

	GetWorld()->SweepSingleByChannel(Hit, StartLoc, EndLoc, _CachedCapsule->GetComponentQuat(), ECC_Pawn, Shape, Params);
	
	/*if (Hit.bBlockingHit)
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint + FVector(0, 0, _CachedCapsule->GetScaledCapsuleRadius()), _CachedCapsule->GetScaledCapsuleRadius(), 32, FColor::Red, false, 2.5f);
	else
		DrawDebugSphere(GetWorld(), EndLoc, _CachedCapsule->GetScaledCapsuleRadius(), 32, FColor::Blue, false, 2.5f);*/

	if (!Hit.bBlockingHit)
		return false;

	if (Hit.Location.Z > _CachedCapsule->GetComponentLocation().Z && !IsWalkableSurface(Hit))
		return false;

	if (Hit.Location.Z != _CachedCapsule->GetComponentLocation().Z)
	{
		/*FVector CurrentLoc = _CachedCapsule->GetComponentLocation();
		CurrentLoc.Z = Hit.Location.Z + 0.5f;
		_CachedCapsule->SetWorldLocation(CurrentLoc, true);
		_CachedCapsule->AddWorldOffset(_Velocity * DeltaTime, true);*/ 
		return true;
	}

	return true;
}

void UVRPawnComponent::UpdateXYVelocity(float DeltaTime, FVector DesiredDirection, float DesiredSpeed)
{
	float CurrentSpeed = _Velocity.Size();
	FVector CurrentDir = _Velocity.GetSafeNormal();

	float FinalSpeed = CurrentSpeed;
	FVector FinalDir = CurrentDir;

	float NewDesiredSpeed = DesiredSpeed;

	if (CurrentSpeed > NewDesiredSpeed)
	{
		FinalSpeed -= _Decceleration * DeltaTime;
		FinalSpeed = FMath::Clamp(FinalSpeed, NewDesiredSpeed, CurrentSpeed);
	}
	else if (CurrentSpeed < NewDesiredSpeed)
	{
		FinalSpeed += _Acceleration * DeltaTime;
		FinalSpeed = FMath::Clamp(FinalSpeed, CurrentSpeed, NewDesiredSpeed);
	}

	FVector NewDesiredVel = DesiredDirection * FinalSpeed;
	FVector NewCurrentVel = CurrentDir * FinalSpeed;
	_Velocity = NewDesiredVel;//FMath::Lerp(NewCurrentVel, NewDesiredVel, _DirectionChangeSpeed);
}

void UVRPawnComponent::ChangeMovementMode(EMovementModes NewMovementMode)
{
	switch (NewMovementMode)
	{
	case EMovementModes::EMM_Grounded:
		_Velocity.Z = 0;
		break;
	case EMovementModes::EMM_Falling:
		break;
	case EMovementModes::EMM_RampSliding:
		_Velocity = FVector::ZeroVector;
		break;
	default:
		break;
	}

	_MovementState = NewMovementMode;
}

void UVRPawnComponent::HandleRampSliding(float DeltaTime)
{
	if (CheckForJump())
		return;

	FHitResult FloorHit;
	DoFloorCheck(FloorHit);

	if (LastGroundCheckCausedMovementStateChange())
		return;

	FVector Dir = FVector(_LastFloorHit._FloorHitNormal.X, _LastFloorHit._FloorHitNormal.Y, 0).GetSafeNormal();
	float FloorDotP = _LastFloorHit._FloorHitNormal | Dir;
	Dir.Z = -FloorDotP / _LastFloorHit._FloorHitNormal.Z;

	// since we have hit a slope, we need to change the velocity and its direction to match the floor we are currently on
	//when sliding down a ramp, we dont care about the X or Y velocity and use the Z value for sliding
	float VelMag = _RunSpeed;// *2 * (1 - _LastFloorHit._FloorHitNormal.Z);

	UpdateXYVelocity(DeltaTime, Dir.GetSafeNormal(), VelMag);
	MoveCapsule(_Velocity.GetSafeNormal(), _Velocity.Size(), DeltaTime, false);
}

bool UVRPawnComponent::CheckForJump()
{
	if (_InJump)
	{
		_Velocity.Z = _JumpZVel;
		ChangeMovementMode(EMovementModes::EMM_Falling);
		return true;
	}
	return false;
}

bool UVRPawnComponent::LastGroundCheckCausedMovementStateChange()
{
	EMovementModes ShouldBeState;
	if (_LastFloorHit._bBlockingHit)
	{
		if(_LastFloorHit._WalkableSurface)
			ShouldBeState = EMovementModes::EMM_Grounded;
		else
			ShouldBeState = EMovementModes::EMM_RampSliding;
	}
	else 
		ShouldBeState = EMovementModes::EMM_Falling;

	if (ShouldBeState != _MovementState)
	{
		ChangeMovementMode(ShouldBeState);
		return true;
	}

	return false;
}

void UVRPawnComponent::SaveClimbingLocation(float DeltaTime)
{
	_ClimbingSaveLocTimer -= DeltaTime;

	if (_ClimbingSaveLocTimer > 0)
		return;

	if (!_CachedCapsule)
		return;

	_ClimbingSaveLocTimer = .05f;

	for (int i = _CachedClimbingMoves.Num() - 1; i > 0; i--)
	{
		//just incase we get to 0
		if (i > 0)
		{
			_CachedClimbingMoves[i] = _CachedClimbingMoves[i - 1];
		}
	}

	_CachedClimbingMoves[0] = _CachedCapsule->GetComponentLocation();
}

FVector UVRPawnComponent::GetClimbingFlingDirection()
{
	float TotalMoveDistance = 0;
	FVector TotalDirections = FVector::ZeroVector;

	for (int i = 0; i < _CachedClimbingMoves.Num(); i++)
	{
		if (i != 0)
		{
			TotalMoveDistance += FVector::DistSquared(_CachedClimbingMoves[0], _CachedClimbingMoves[i]);
			TotalDirections += FVector(_CachedClimbingMoves[0] - _CachedClimbingMoves[i]).GetSafeNormal();
		}
	}

	if (TotalMoveDistance < 10)
		return FVector::ZeroVector;

	TotalDirections.Normalize();
	FVector FinalFlingDir = FVector::ZeroVector;
	float CurrenttDotP = 0;

	for (int i = 0; i < _ClimbingFlingAllowedDirections.Num(); i++)
	{
		float DotP = FVector::DotProduct(TotalDirections, _ClimbingFlingAllowedDirections[i]);
		
		if (DotP > CurrenttDotP)
		{
			CurrenttDotP = DotP;
			FinalFlingDir = _ClimbingFlingAllowedDirections[i];
		}
	}

	return FinalFlingDir;
}

void UVRPawnComponent::NetMulticast_SendMove_Implementation(FVector NewLocation)
{
	_LastRecievedLocation = NewLocation;
}

void UVRPawnComponent::Server_SendMove_Implementation(FVector NewLocation)
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Green, "NewMove");
		GEngine->AddOnScreenDebugMessage(2, 1.5f, FColor::Green, "New Location: " + NewLocation.ToString());
	}*/

	NetMulticast_SendMove(NewLocation);
}