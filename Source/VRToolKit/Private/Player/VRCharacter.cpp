// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRCharacter.h"	
#include "Player/VRPawnComponent.h"	
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Online/NetworkHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Player/VRHand.h"
#include "Components/SkeletalMeshComponent.h"
#include "Player/VRCharacterAB.h"
#include "MotionControllerComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Components/SphereComponent.h"

#define LEVEL_NETWORKTEST TEXT("/Game/Content/Levels/TestLevels/LVL_NetworkTest");
#define GameMode_MP_Default TEXT("?listen?game=Default");

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Setup VRPawnComp
	_VRPawnComp = CreateDefaultSubobject<UVRPawnComponent>("PawnComp");

	//Setup CharacterRoot
	_CharacterRoot = CreateDefaultSubobject<USceneComponent>("CharacterRoot");
	SetRootComponent(_CharacterRoot);

	//Setup CharacterCapsule
	_CharacterCap = CreateDefaultSubobject<UCapsuleComponent>("CharacterCap");
	_CharacterCap->SetupAttachment(GetRootComponent());
	_CharacterCap->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	//Setup Character camera
	_CharacterCam = CreateDefaultSubobject<UCameraComponent>("CharacterCam");
	_CharacterCam->SetupAttachment(GetRootComponent());

	//Setup HMDMesh
	_HMDMesh = CreateDefaultSubobject<UStaticMeshComponent>("HMDMesh");
	_HMDMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_HMDMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	_HMDMesh->SetWorldScale3D(FVector(1.5f, 1.5f, 1.5f));
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> HMDStaticMesh(
			TEXT("/Engine/VREditor/Devices/Generic/GenericHMD"));
		if (HMDStaticMesh.Succeeded())
		{
			_HMDMesh->SetStaticMesh(HMDStaticMesh.Object);
			_HMDMesh->SetupAttachment(_CharacterCam);
		}
	}

	//Setup VestMesh
	_VestMesh = CreateDefaultSubobject<UStaticMeshComponent>("VestMesh");
	_VestMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_VestMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	_VestMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
	/* {
		static ConstructorHelpers::FObjectFinder<UStaticMesh> VestStaticMesh(
			TEXT("/Game/Content/Meshes/Vest"));
		if (VestStaticMesh.Succeeded())
		{
			_VestMesh->SetStaticMesh(VestStaticMesh.Object);
			_VestMesh->SetupAttachment(_CharacterRoot);
		}
	}*/

	//Setup VRBody
	_VRBody = CreateDefaultSubobject<USkeletalMeshComponent>("VRBody");
	_VRBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_VRBody->SetupAttachment(_CharacterRoot);

	//SetupMotionControllers
	_LeftHandRoot = CreateDefaultSubobject<USceneComponent>("LeftHandRoot");
	_LeftHandRoot->SetupAttachment(_CharacterRoot);

	_LeftHandMCComp = CreateDefaultSubobject<UMotionControllerComponent>("LeftHandMCComp");
	_LeftHandMCComp->SetupAttachment(_LeftHandRoot);
	_LeftHandMCComp->MotionSource = "Left";

	_RightHandRoot = CreateDefaultSubobject<USceneComponent>("RightHandRoot");
	_RightHandRoot->SetupAttachment(_CharacterRoot);

	_RightHandMCComp = CreateDefaultSubobject<UMotionControllerComponent>("RightHandMCComp");
	_RightHandMCComp->SetupAttachment(_RightHandRoot);
	_RightHandMCComp->MotionSource = "Right";

	//ClimbingZone
	_ClimbingZoneDetection = CreateDefaultSubobject<USphereComponent>("ClimbingDetectionZone");
	_ClimbingZoneDetection->SetupAttachment(_CharacterCap);
	_ClimbingZoneDetection->SetSphereRadius(75.f);
	_ClimbingZoneDetection->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	_ClimbingZoneDetection->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (_VRPawnComp)
	{
		_VRPawnComp->SetCachedComponents(_CharacterCap, _CharacterCam, _CharacterRoot, _ClimbingZoneDetection);
	}

	if (IsLocallyControlled() && _HMDMesh)
	{
		_HMDMesh->SetVisibility(false);
		Server_AskForHands();
	}
	else if (_HMDMesh)
	{
		_HMDMesh->SetVisibility(true);
	}

	if (GetLocalRole() >= ENetRole::ROLE_Authority)
	{
		SpawnHands();	
	}

	if (_VRBody)
	{
		_CharacterAB = Cast<UVRCharacterAB>(_VRBody->GetAnimInstance());
	}
}

void AVRCharacter::Server_AskForHands_Implementation()
{
	NetMulticast_SetHands(_LeftHand, _RightHand);
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (_LeftHand && _RightHand && GetLocalRole() >= ENetRole::ROLE_Authority)
	{
		_LeftHand->SetOwner(GetOwner());
		_RightHand->SetOwner(GetOwner());
	}

	if (IsLocallyControlled())
	{
		_ReplicatedHMDTransformTimer -= DeltaTime;

		if (_ReplicatedHMDTransformTimer <= 0 && _CharacterCam)
		{
			_ReplicatedHMDTransformTimer = 0.1f;
			Server_NewHMDTransform(_CharacterCam->GetComponentLocation(), 
				_CharacterCam->GetComponentRotation());
			//Server_AskForHands();
		}

		bool UsingHMD = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

		if (!UsingHMD && _LeftHandRoot && _RightHandRoot && _CharacterCam)
		{
			FVector Start = _CharacterCam->GetComponentLocation();
			FVector ForwardDir = _CharacterCam->GetForwardVector();
			FVector RightDir = _CharacterCam->GetRightVector();

			_LeftHandRoot->SetWorldLocation(Start + (ForwardDir * 75));
			_LeftHandRoot->AddWorldOffset((RightDir * -20));
			_LeftHandRoot->SetWorldRotation(_CharacterCam->GetComponentRotation());

			_RightHandRoot->SetWorldLocation(Start + (ForwardDir * 75));
			_RightHandRoot->AddWorldOffset((RightDir * 20));
			_RightHandRoot->SetWorldRotation(_CharacterCam->GetComponentRotation());
		}

		{
			FVector NewLocation;
			FVector CurrentLoc = _VestMesh->GetComponentLocation();
			FVector GotoLoc = _CharacterCam->GetComponentLocation() - (_VestMesh->GetRightVector() * 12);
			NewLocation.X = FMath::FInterpTo(CurrentLoc.X, GotoLoc.X, DeltaTime, 150.f);
			NewLocation.Y = FMath::FInterpTo(CurrentLoc.Y, GotoLoc.Y, DeltaTime, 150.f);
			NewLocation.Z = FMath::FInterpTo(CurrentLoc.Z, GotoLoc.Z - 175, DeltaTime, 150.f);
			_VestMesh->SetWorldLocation(NewLocation);

			FRotator NewRotation;
			FRotator CurrentRot = _VestMesh->GetComponentRotation();
			FRotator NewRot = _CharacterCam->GetComponentRotation();
			NewRot.Pitch = 0;
			NewRot.Yaw -= 90;
			NewRot.Roll = 0;
			NewRotation = FMath::RInterpTo(CurrentRot, NewRot, DeltaTime, 50.f);
			_VestMesh->SetWorldRotation(NewRotation);
		}
	}
	else if(GetLocalRole() >= ENetRole::ROLE_SimulatedProxy && _HMDMesh)
	{
		{
			FVector NewLocation;
			FVector CurrentLoc = _HMDMesh->GetComponentLocation();
			NewLocation.X = FMath::FInterpTo(CurrentLoc.X, _LastKNownHMDLocation.X, DeltaTime, 15.f);
			NewLocation.Y = FMath::FInterpTo(CurrentLoc.Y, _LastKNownHMDLocation.Y, DeltaTime, 15.f);
			NewLocation.Z = FMath::FInterpTo(CurrentLoc.Z, _LastKNownHMDLocation.Z, DeltaTime, 15.f);
			_HMDMesh->SetWorldLocation(NewLocation);
		}

		{
			FVector NewLocation;
			FVector CurrentLoc = _VestMesh->GetComponentLocation();
			NewLocation.X = FMath::FInterpTo(CurrentLoc.X, _LastKNownHMDLocation.X, DeltaTime, 15.f);
			NewLocation.Y = FMath::FInterpTo(CurrentLoc.Y, _LastKNownHMDLocation.Y, DeltaTime, 15.f);
			NewLocation.Z = FMath::FInterpTo(CurrentLoc.Z, _LastKNownHMDLocation.Z - 100, DeltaTime, 15.f);
			_VestMesh->SetWorldLocation(NewLocation);
		}

		FRotator NewRotation;
		FRotator CurrentRot = _HMDMesh->GetComponentRotation();
		NewRotation = FMath::RInterpTo(CurrentRot, _LastKNownHMDRotation, DeltaTime, 10.f);
		_HMDMesh->SetWorldRotation(NewRotation);

		FRotator VNewRot = _CharacterCam->GetComponentRotation();
		VNewRot.Pitch = 0;
		VNewRot.Roll = 0;
		NewRotation = FMath::RInterpTo(CurrentRot, VNewRot, DeltaTime, 10.f);
		_VestMesh->SetWorldRotation(NewRotation);

	}

	VRBodyTick(DeltaTime);
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MC_Thumbstick(L)_Y", this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MC_Thumbstick(L)_X", this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MC_Thumbstick(R)_X", this, &AVRCharacter::YawRotation);
	PlayerInputComponent->BindAxis("MC_Thumbstick(R)_Y", this, &AVRCharacter::PitchRotation);
	PlayerInputComponent->BindAxis("LeftGripPressed", this, &AVRCharacter::LeftGripPressed);
	PlayerInputComponent->BindAxis("RightGripPressed", this, &AVRCharacter::RightGripPressed);
	PlayerInputComponent->BindAxis("LeftTriggerPressed", this, &AVRCharacter::LeftTriggerPressed);
	PlayerInputComponent->BindAxis("RightTriggerPressed", this, &AVRCharacter::RightTriggerPressed);

	PlayerInputComponent->BindAction("HostServer", IE_Pressed, this, &AVRCharacter::HostServer);
	PlayerInputComponent->BindAction("JoinServer", IE_Pressed, this, &AVRCharacter::JoinServer);
	PlayerInputComponent->BindAction("SwitchVRMode", IE_Pressed, this, &AVRCharacter::SwitchVRMode);
	PlayerInputComponent->BindAction("RescaleVRBody", IE_Pressed, this, &AVRCharacter::RescaleVRBody);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AVRCharacter::DoJump);
	PlayerInputComponent->BindAction("MC_X_Press", IE_Pressed, this, &AVRCharacter::LeftBottomButtonPressed);
	PlayerInputComponent->BindAction("MC_Y_Press", IE_Pressed, this, &AVRCharacter::LeftTopButtonPressed);
	PlayerInputComponent->BindAction("MC_A_Press", IE_Pressed, this, &AVRCharacter::RightBottomButtonPressed);
	PlayerInputComponent->BindAction("MC_B_Press", IE_Pressed, this, &AVRCharacter::RightTopButtonPressed);
}

void AVRCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!_LeftHand || !_RightHand)
		return;

	//_LeftHand->SetOwner(NewController);
	//_RightHand->SetOwner(NewController);
	_LeftHand->UpdateOwner(NewController);
	_RightHand->UpdateOwner(NewController);

	NetMulticast_SetHands(_LeftHand, _RightHand);
	NetMulticast_SetNewOwner(NewController);
}

void AVRCharacter::MoveForward(float Value)
{
	if (Value < 0.15f && Value > -0.15f)
		return;

	if (!_CharacterCap)
		return;

	AddMovementInput(_CharacterCap->GetForwardVector(), Value);

}

void AVRCharacter::MoveRight(float Value)
{
	if (Value < 0.15f && Value > -0.15f)
		return;

	if (!_CharacterCap)
		return;

	AddMovementInput(_CharacterCap->GetRightVector(), Value);
}

void AVRCharacter::YawRotation(float Value)
{
	AddRotationInput(FRotator(0, Value, 0));
}

void AVRCharacter::PitchRotation(float Value)
{
	AddRotationInput(FRotator(Value, 0, 0));
}

void AVRCharacter::HostServer()
{
	FName Map = LEVEL_NETWORKTEST;
	FString Mode = GameMode_MP_Default;
	UGameplayStatics::OpenLevel(GetWorld(), Map, true, Mode);
}

void AVRCharacter::JoinServer()
{
	FString Open = "open 86.7.212.96";
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->ConsoleCommand(Open);
}

void AVRCharacter::SwitchVRMode()
{
	UHeadMountedDisplayFunctionLibrary::EnableHMD(true);
}

void AVRCharacter::LeftGripPressed(float Value)
{
	if (!_LeftHand)
		return;

	if (Value > 0.35f)
	{
		_LeftHand->GripPressed();
	}
	else
	{
		_LeftHand->GripReleased();
	}
}

void AVRCharacter::RightGripPressed(float Value)
{ 
	if (!_RightHand)
		return;

	if (Value > 0.35f)
	{
		_RightHand->GripPressed();
	}
	else
	{
		_RightHand->GripReleased();
	}
}

void AVRCharacter::LeftTriggerPressed(float Value)
{
	if (!_LeftHand)
		return;

	_LeftHand->TriggerPressed(Value);
}

void AVRCharacter::RightTriggerPressed(float Value)
{
	if (!_RightHand)
		return;

	_RightHand->TriggerPressed(Value);
}

void AVRCharacter::LeftTopButtonPressed()
{
	if (!_LeftHand)
		return;

	_LeftHand->TopButtonPressed(true);
}

void AVRCharacter::LeftBottomButtonPressed()
{
	if (!_LeftHand)
		return;

	_LeftHand->BottomButtonPressed(true);


}

void AVRCharacter::RightTopButtonPressed()
{
	if (!_RightHand)
		return;

	_RightHand->TopButtonPressed(true);
}

void AVRCharacter::RightBottomButtonPressed()
{
	if (!_RightHand)
		return;

	_RightHand->BottomButtonPressed(true);
}

void AVRCharacter::DoJump()
{
	if (!_VRPawnComp)
		return;

	GEngine->AddOnScreenDebugMessage(51, 5.0f, FColor::Red, "DoJumpChar");
	_VRPawnComp->DoJump();
	
}

void AVRCharacter::AddMovementInput(FVector Direction, float Scale)
{
	if (!_VRPawnComp)
		return;

	_VRPawnComp->AddMovementInput(Direction, Scale);
}

void AVRCharacter::AddRotationInput(FRotator Rotation)
{
	if (!_VRPawnComp)
		return;

	_VRPawnComp->AddRotationInput(Rotation);
}

void AVRCharacter::SpawnHands()
{
	if (!_BP_DefaultHand)
		return;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();

	FAttachmentTransformRules HandRule = FAttachmentTransformRules::SnapToTargetIncludingScale;
	bool UsingHMD = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

	if (!_LeftHand)
	{
		_LeftHand = GetWorld()->SpawnActor<AVRHand>(_BP_DefaultHand, GetActorLocation(), GetActorRotation(), SpawnInfo);
		//_LeftHand->AttachToComponent(GetRootComponent(), HandRule);
		_LeftHand->IsRightHand(false, _LeftHandMCComp);
		_LeftHand->SetCharacterAttachedTo(this);

		if (!UsingHMD)
			_LeftHand->NonVRFollow(_LeftHandRoot);
	}

	if (!_RightHand)
	{
		_RightHand = GetWorld()->SpawnActor<AVRHand>(_BP_DefaultHand, GetActorLocation(), GetActorRotation(), SpawnInfo);
		//_RightHand->AttachToComponent(GetRootComponent(), HandRule);
		_RightHand->IsRightHand(true, _RightHandMCComp);
		_RightHand->SetCharacterAttachedTo(this);

		if (!UsingHMD)
			_RightHand->NonVRFollow(_RightHandRoot);
	}
}

void AVRCharacter::VRBodyTick(float DeltaTime)
{
	if (!_VRBody || !_CharacterCam)
		return;
	
	FTransform C_Transform = _CharacterCam->GetComponentTransform();
	FTransform B_Transform = _VRBody->GetComponentTransform();

	FRotator NewRot = B_Transform.GetRotation().Rotator();
	NewRot.Yaw = C_Transform.GetRotation().Rotator().Yaw - 90;
	_VRBody->SetWorldRotation(NewRot);

	FVector NewLoc = C_Transform.GetLocation();
	NewLoc.Z = GetActorLocation().Z;//_PlayersHeight;
	NewLoc -= _VRBody->GetRightVector() * 18;
	_VRBody->SetWorldLocation(NewLoc);

	if (!_CharacterAB)
		return;

	if (_LeftHand)
	{
		FTransform T = _LeftHand->GetPhysicsObjectTransform();
		_CharacterAB->SetLeftHandLocationAndRotation(T.GetLocation(), T.GetRotation().Rotator());
	}

	if (_RightHand)
	{
		FTransform T = _RightHand->GetPhysicsObjectTransform();
		_CharacterAB->SetRightHandLocationAndRotation(T.GetLocation(), T.GetRotation().Rotator());
	}
}

void AVRCharacter::RescaleVRBody()
{
	if (!_CharacterCam || !_VRBody)
		return;

	float PlayerHeight = _CharacterCam->GetComponentLocation().Z - GetActorLocation().Z;
	_PlayersHeight = PlayerHeight;
	float NewScale = (1.0f / 180.0f) * PlayerHeight;
	_VRBody->SetWorldScale3D(FVector(NewScale));
}

FTransform AVRCharacter::GetVRCameraTransorm()
{
	if (!_CharacterCam)
		return FTransform();

	return _CharacterCam->GetComponentTransform();
}

FVector AVRCharacter::GetCollisionLocation()
{
	if (!_CharacterCap)
		return FVector::ZeroVector;

	return _CharacterCap->GetComponentLocation();
}

void AVRCharacter::HandGrabbedClimbingPoint(bool LeftHand, FQuat ClimbingHandQuat, EClimbingMode AttemptedClimbingMode)
{
	_VRPawnComp->HandGrabbedClimbingPoint(LeftHand, ClimbingHandQuat, AttemptedClimbingMode);
}

FClimbingHandInfo AVRCharacter::GetLeftHandClimbInfo()
{
	if(!_LeftHand)
		return FClimbingHandInfo();

	return _LeftHand->GetClimbingHandInfo();
}

FClimbingHandInfo AVRCharacter::GetRightHandClimbInfo()
{
	if (!_RightHand)
		return FClimbingHandInfo();

	return _RightHand->GetClimbingHandInfo();
}

const FVector AVRCharacter::GetDesiredVelocity()
{
	if (_VRPawnComp)
	{
		return _VRPawnComp->GetVelocity();
	}

	return FVector::ZeroVector;
}

FVector AVRCharacter::GetCharacterCollisionLocation()
{
	if (_CharacterCap) 
		return _CharacterCap->GetComponentLocation();

	return FVector::ZeroVector;
}

void AVRCharacter::NetMulticast_SetNewOwner_Implementation(AController* NewController)
{
	if (!_LeftHand || !_RightHand)
		return;

	//_LeftHand->SetOwner(NewController);
	//_RightHand->SetOwner(NewController);

	_LeftHand->UpdateOwner(NewController);
	_RightHand->UpdateOwner(NewController);
}

void AVRCharacter::NetMulticast_SetHands_Implementation(AVRHand* LeftHand, AVRHand* RightHand)
{
	if (!LeftHand || !RightHand)
		return;

	_LeftHand = LeftHand;
	_RightHand = RightHand;

	if (_LeftHand && _RightHand)
	{
		_LeftHand->SetActorLocation(GetActorLocation(), false, nullptr, ETeleportType::ResetPhysics);
		_LeftHand->IsRightHand(false, _LeftHandMCComp);

		_RightHand->SetActorLocation(GetActorLocation(), false, nullptr, ETeleportType::ResetPhysics);
		_RightHand->IsRightHand(true, _RightHandMCComp);
	}
}

void AVRCharacter::Server_NewHMDTransform_Implementation(FVector Location, FRotator Rotation)
{
	_LastKNownHMDLocation = Location;
	_LastKNownHMDRotation = Rotation;
}

void AVRCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVRCharacter, _LastKNownHMDLocation);
	DOREPLIFETIME(AVRCharacter, _LastKNownHMDRotation);
}

