// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRHand.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "MotionControllerComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Online/NetworkHelpers.h"
#include "Items/ItemGrabComponent.h"
#include "DrawDebugHelpers.h"
#include "Items/VRItem.h"
#include "Online/PhysicsReplicationComponent.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Components/SphereComponent.h"
#include "ClimbingSystem/EnvironmentGrabComponent.h"
#include "Player/VRCharacter.h"
#include "Utility/ExtraMaths.h"
#include "Components/BoxComponent.h"
#include "Player/VRPawnComponent.h"
#include "Weapons/AmmoStorageComponent.h"
#include "Sound/SoundCue.h"
#include "Items/ItemStorer.h"

// Sets default values
AVRHand::AVRHand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	//Motion Controller Component
	/*_MotionControllerComp = CreateDefaultSubobject<UMotionControllerComponent>("MotionController");
	_MotionControllerComp->SetupAttachment(GetRootComponent());*/

	//Physics Hand Collision
	{
		_ControllerSKM = CreateDefaultSubobject<USkeletalMeshComponent>("PhysicsHandCollision");
		SetRootComponent(_ControllerSKM);
		_ControllerSKM->SetupAttachment(GetRootComponent());
		_ControllerSKM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		_ControllerSKM->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		_ControllerSKM->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		_ControllerSKM->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
		_ControllerSKM->SetSimulatePhysics(true);
	}

	//VR Controller Mesh
	/* {
		_ControllerSM = CreateDefaultSubobject<UStaticMeshComponent>("ControllerMesh");
		_ControllerSM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		_ControllerSM->SetupAttachment(_MotionControllerComp);

		_LeftPhysicsHandSM = CreateDefaultSubobject<UStaticMeshComponent>("LeftPhysicsHandSM");
		_LeftPhysicsHandSM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		_LeftPhysicsHandSM->SetupAttachment(_PhysicsHandCollision);

		_RightPhysicsHandSM = CreateDefaultSubobject<UStaticMeshComponent>("RightPhysicsHandSM");
		_RightPhysicsHandSM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		_RightPhysicsHandSM->SetupAttachment(_PhysicsHandCollision);

		static ConstructorHelpers::FObjectFinder<UStaticMesh> ControllerStaticMesh(
			TEXT("/Engine/VREditor/Devices/Oculus/OculusControllerMesh"));
		if (ControllerStaticMesh.Succeeded())
		{
			_ControllerSM->SetStaticMesh(ControllerStaticMesh.Object);
			_LeftPhysicsHandSM->SetStaticMesh(ControllerStaticMesh.Object);
			_RightPhysicsHandSM->SetStaticMesh(ControllerStaticMesh.Object);
			
		}

		/*static ConstructorHelpers::FObjectFinder<UMaterialInstance> ControllerMaterial(
			TEXT("/Game/Content/Materials/Colours/M_Translucent_Blue"));
		if (ControllerMaterial.Succeeded())
		{
			_ControllerSM->SetMaterial(0, ControllerMaterial.Object);
		}
	}*/

	_PRC = CreateDefaultSubobject<UPhysicsReplicationComponent>("PRC");

	_PHC = CreateDefaultSubobject<UPhysicsHandlerComponent>("PHC");

	//Grab Sphere
	{
		_GrabSphere = CreateDefaultSubobject<USphereComponent>("GrabSphere");
		_GrabSphere->SetupAttachment(_ControllerSKM);
		_GrabSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		_GrabSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		_GrabSphere->SetSphereRadius(5.f);
		_GrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRHand::GrabSphereOverlapBegin);
		_GrabSphere->OnComponentEndOverlap.AddDynamic(this, &AVRHand::GrabSphereOverlapEnd);
	}
}


// Called when the game starts or when spawned
void AVRHand::BeginPlay()
{
	Super::BeginPlay();


	//_PRC->SetReplicatedPhysicsObject(_PhysicsHandCollision);

	//This is gonna be all zero which will cause the hand to flip out if we dont set it
	_MC_LastLoc = GetActorLocation();
}

void AVRHand::OnRep_Owner()
{
	if (GetOwner() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		GEngine->AddOnScreenDebugMessage(982, 15.f, FColor::Black, "Requested character from server");
		NF_Server_RequestCharacterOwner();
	}
}

// Called every frame
void AVRHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_GripCheckTimer > 0 && !_GrabbedEGC && !_DynamicGrabbed_ActorGrabbed && !_Vaulting_Actor && !_ComponentHeld)
	{
		_GripCheckTimer -= DeltaTime;
		GripPressCheck();
	}

	bool UsingHMD = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

	if (_PHC)
	{
		if (_CharacterAttachedTo && (_GrabbedEGC || _DynamicGrabbed_ActorGrabbed || _Vaulting_Actor))
		{
			UpdateHandClimbInfo();
			FTransform FinalHandLoc = _ClimbingGrabTransform;
			FinalHandLoc.SetLocation(FinalHandLoc.GetLocation() + _ClimbingInfo._EGC_MoveDiff);
			_ControllerSKM->SetWorldTransform(FinalHandLoc);
			_ClimbingInfo._HandQuat = GetActorQuat();
			//_PHC->SetTargetTransform(FinalHandLoc);
			//_PHC->SetTargetLocationOffset(FVector::ZeroVector);
			//_PHC->SetTargetRotationOffset(FQuat(FRotator(0, 0, 0)));
		}
		else
		{
			FVector FinalOffset = FVector::ZeroVector;
			FQuat HandRot;
			FQuat VerticalOffset;

			if (UsingHMD)
			{
				HandRot = GetTrackingHandTransform().GetRotation();
				HandRot *= FQuat(FRotator(-45, 0, 0));
				FinalOffset += HandRot.GetForwardVector() * _HandOffset.X;
				FinalOffset += HandRot.GetRightVector() * _HandOffset.Y;
				FinalOffset += HandRot.GetUpVector() * _HandOffset.Z;

				VerticalOffset = FQuat(FRotator(0, 0, 180));

				if (_bRightHand)
					VerticalOffset = FQuat(FRotator(135, 0, 180));
			}
			else
			{
				FinalOffset += GetActorForwardVector() * _HandOffset.X;
				FinalOffset += GetActorRightVector() * _HandOffset.Y;
				FinalOffset += GetActorUpVector() * _HandOffset.Z;

				VerticalOffset = FQuat(FRotator(180, 0, 0));

				if (_bRightHand)
					VerticalOffset = FQuat(FRotator(180, 0, 180));
			}

			_PHC->SetTargetRotationOffset(VerticalOffset);
			_PHC->SetTargetLocationOffset(FinalOffset);

			if (_CharacterAttachedTo)
			{				
				_PHC->SetDesiredVelocity(_CharacterAttachedTo->GetDesiredVelocity());
			}
		}
	}
}

void AVRHand::MoveHandOutsideOfHeldObject()
{
	if (!_MotionControllerComp || !_ControllerSKM)
		return;

	/*FVector EndLoc = _MotionControllerComp->GetComponentLocation();
	FVector StartLoc = _ControllerSKM->GetComponentLocation() + (GetMotionControllerMoveForwardDir() * 10);
	StartLoc.Z = EndLoc.Z;
	FVector Dir = EndLoc - StartLoc;
	ExtraMaths::CorrectNormalizedVector(Dir);
	EndLoc -= Dir * 10;

	_ControllerSKM->SetWorldLocation(StartLoc);
	_ControllerSKM->SetWorldLocation(EndLoc, true);
	_ControllerSKM->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	_ControllerSKM->SetPhysicsLinearVelocity(FVector::ZeroVector);*/
}

void AVRHand::UpdateHandClimbInfo()
{
	if (!_MotionControllerComp)
		return;

	if (_GrabbedEGC)
	{
		_ClimbingInfo._EGC_CurrentLoc = _GrabbedEGC->GetComponentLocation();
	}
	else if (_DynamicGrabbed_ActorGrabbed)
	{
		_ClimbingInfo._EGC_CurrentLoc = _DynamicGrabbed_ActorGrabbed->GetActorLocation();
	}
	else if (_Vaulting_Actor)
	{
		_ClimbingInfo._EGC_CurrentLoc = _Vaulting_Actor->GetActorLocation();
	}

	_ClimbingInfo._EGC_MoveDiff = _ClimbingInfo._EGC_CurrentLoc - _ClimbingInfo._EGC_StartLocation;
	_ClimbingInfo._Hand_CurrentLoc = GetActorLocation();
	_ClimbingInfo._MC_Location = _MotionControllerComp->GetComponentLocation();
	_ClimbingInfo._Hand_MoveDiff = _MotionControllerComp->GetComponentLocation() - (_ClimbingInfo._GrabbedLocation + _ClimbingInfo._EGC_MoveDiff);

}

bool AVRHand::CheckDynamicEdgeGrap()
{
	if (!_ControllerSKM)
		return false;

	FQuat ActorQuat = GetActorQuat();

	FHitResult ParmHit;
	FHitResult FingersHit;
	FCollisionQueryParams QParams;
	QParams.bTraceComplex = false;
	FRotator ActorRot = ActorQuat.Rotator();
	FQuat FingerRot = FQuat(FRotator(0, ActorRot.Yaw, 0));

	//scale it based on the rotation of the hand
	float Angle = ExtraMaths::GetAngleOfTwoVectors(ActorQuat.GetForwardVector(), FVector(0, 0, 1));
	Angle = FMath::Clamp(Angle, 0, 90);

	float ZParmSize = _ParmEdgeDetectBoxExtent.Z - (_ParmEdgeDetectBoxExtent.Z * (((1.f / 90.f) * Angle)));
	ZParmSize = FMath::Clamp(ZParmSize, _MinZSizeOfParmCheck, _ParmEdgeDetectBoxExtent.Z);
	FVector3f ParmBoxSize = FVector3f(_ParmEdgeDetectBoxExtent.X, _ParmEdgeDetectBoxExtent.Y, ZParmSize);

	//Check to see if our parm hits anything
	FVector ParmHitStart = GetActorLocation(); /*+ (FVector::UpVector * (ZParmSize - (_MaxSizeCheckUnderParm / 2)))*/;
	FVector ForwardDir = GetActorForwardVector();
	//ForwardDir.Z = 0;
	//ForwardDir.Normalize();
	FVector ParmHitEnd = ParmHitStart + (ForwardDir * 20);
	FCollisionShape ParmTraceShape;

	ParmTraceShape.SetBox(ParmBoxSize + FVector3f(0, 0, _MaxSizeCheckUnderParm / 2));
	GetWorld()->SweepSingleByChannel(ParmHit, ParmHitStart, ParmHitEnd, FingerRot, ECollisionChannel::ECC_Pawn, ParmTraceShape, QParams);

	bool bEdgeGrabbed = false;
	//if our parm hits something then their is a chance of a ledge grab
	if (ParmHit.IsValidBlockingHit())
	{
		if (AVRItem* Item = Cast<AVRItem>(ParmHit.GetActor()))
		{
			return false;
		}

		//DrawDebugBox(GetWorld(), ParmHit.Location, ParmTraceShape.GetBox(), FingerRot, FColor::Green, false, 1.f);

		//Check if our fingers hit anything
		float ZHeightCheck = ZParmSize + _FingersEdgeDetectBoxExtent.Z + (_MaxSizeCheckUnderParm);
		FVector FingersHitStart = ParmHit.Location + FVector(0, 0, ZHeightCheck);
		FVector FingersHitEnd = FingersHitStart - FVector(0, 0, ZHeightCheck * 2);
		FCollisionShape FingersTraceShape;
		FingersTraceShape.SetBox(_FingersEdgeDetectBoxExtent);

		GetWorld()->SweepSingleByChannel(FingersHit, FingersHitStart, FingersHitEnd, FingerRot,
			ECollisionChannel::ECC_Pawn, FingersTraceShape, QParams);

		//if our fingers hit then their is still a change of a ledge grab
		if (FingersHit.IsValidBlockingHit())
		{

			//DrawDebugBox(GetWorld(), FingersHit.Location, FingersTraceShape.GetBox(), FingerRot, FColor::Red, false, 1.f);

			float ParmHitAngle = ExtraMaths::GetAngleOfTwoVectors(ParmHit.ImpactNormal, FVector::UpVector);
			float FingersHitAngle = ExtraMaths::GetAngleOfTwoVectors(FingersHit.ImpactNormal, FVector::UpVector);
			
			if (ParmHitAngle > _MinSideAngleGrabbable)
			{
				if (FingersHitAngle < _MaxTopAngleGrabbable)
				{
					bEdgeGrabbed = true;
				}
				else if (ExtraMaths::GetAngleOfTwoVectors(FingersHit.ImpactNormal, FingersHit.ImpactNormal) > 90)
				{
					bEdgeGrabbed = true;
				}
			}
			else if(ExtraMaths::GetAngleOfTwoVectors(FingersHit.ImpactNormal, FingersHit.ImpactNormal) > 90)
			{
				bEdgeGrabbed = true;
			}	
		}
		//else
			//DrawDebugBox(GetWorld(), FingersHit.TraceEnd, FingersTraceShape.GetBox(), FingerRot,  FColor::Red, false, 1.f);

		//if we still dont have a valid grab point then check if we can wrap our hand around the object	
		if (!bEdgeGrabbed)
		{
			FingersHitStart = ParmHit.Location + (GetActorUpVector() * ZHeightCheck);
			FingersHitEnd = FingersHitStart - (GetActorUpVector() * ZHeightCheck * 2);
			GetWorld()->SweepSingleByChannel(FingersHit, FingersHitStart, FingersHitEnd, FingerRot,
				ECollisionChannel::ECC_Pawn, FingersTraceShape, QParams);

			if (FingersHit.IsValidBlockingHit())
			{
				//DrawDebugBox(GetWorld(), FingersHit.Location, FingersTraceShape.GetBox(), FingerRot, FColor::Red, false, 1.f);
				FHitResult UnderHandHit;
				FVector UnderHandHitStart = ParmHit.Location + (-FingersHit.ImpactNormal * ZHeightCheck / 2);
				FVector UnderHandHitEnd = UnderHandHitStart + (FingersHit.ImpactNormal * ZHeightCheck / 2);

				GetWorld()->SweepSingleByChannel(UnderHandHit, UnderHandHitStart, UnderHandHitEnd, FingerRot,
					ECollisionChannel::ECC_Pawn, FingersTraceShape, QParams);

				if (UnderHandHit.IsValidBlockingHit() && FingersHit.IsValidBlockingHit() && ExtraMaths::GetAngleOfTwoVectors(UnderHandHit.ImpactNormal, FingersHit.ImpactNormal) > 145)
				{
					//DrawDebugBox(GetWorld(), UnderHandHit.Location, FingersTraceShape.GetBox(), FingerRot, FColor::Red, false, 1.f);
					bEdgeGrabbed = true;
				}
				/*else
					DrawDebugBox(GetWorld(), UnderHandHit.TraceEnd, FingersTraceShape.GetBox(), FingerRot, FColor::Red, false, 1.f);*/
			}
			/*else
				DrawDebugBox(GetWorld(), FingersHit.TraceEnd, FingersTraceShape.GetBox(), FingerRot, FColor::Red, false, 1.f);*/
		}
	}
	//else
		//DrawDebugBox(GetWorld(), ParmHit.TraceEnd, ParmTraceShape.GetBox(), FingerRot, FColor::Green, false, .1f);


	if (bEdgeGrabbed)
	{
		_DynamicGrabbed_ActorGrabbed = FingersHit.GetActor();


		FVector NewGrabLocation = ParmHit.Location;
		NewGrabLocation.Z = FingersHit.Location.Z;
		NewGrabLocation += (-ParmHit.ImpactNormal * _HandToWallClimbingOffset);
		_ClimbingGrabTransform.SetLocation(NewGrabLocation - FVector(0, 0, _HandHeightClimbingGrabOffset));

		FQuat NewRotation(FRotator(90, 0, 0));

		float YawAngle = ExtraMaths::GetSignedAngleOfTwoVectors(NewRotation.GetUpVector(), ParmHit.ImpactNormal, -NewRotation.GetRightVector());
		FVector YawCrossP = FVector::CrossProduct(NewRotation.GetForwardVector(), ParmHit.ImpactNormal);
		FQuat YawQuatOffset = FQuat(FVector::ForwardVector, FMath::DegreesToRadians(YawAngle));
		NewRotation = NewRotation * YawQuatOffset;

		/*float PitchAngle = ExtraMaths::GetAngleOfTwoVectors(NewRotation.GetForwardVector(), FingersHit.ImpactNormal);
		FVector PitchCrossP = FVector::CrossProduct(NewRotation.GetForwardVector(), FingersHit.ImpactNormal);
		FQuat PitchQuatOffset = FQuat(PitchCrossP, FMath::DegreesToRadians(PitchAngle));
		NewRotation = NewRotation * PitchQuatOffset;*/

		_ClimbingGrabTransform.SetRotation(NewRotation);


		ChangeClimbingMode(EClimbingMode::ECM_GrabbedClimbing);
		return true;
	}

	return false;
}

void AVRHand::CheckForVaulting()
{
	//if we are currently already holding onto a edge/ledge then we dont need to vault
	if (_GrabbedEGC || _DynamicGrabbed_ActorGrabbed || !_MotionControllerComp || !_CharacterAttachedTo)
		return;

	//Check If anything is below our hand
	if (!_Vaulting_Actor)
	{

		//can only vault if our hand is fairly flat on the ground
		float CurrentHandAngle = ExtraMaths::GetAngleOfTwoVectors(GetActorForwardVector(), FVector::UpVector);
		if (CurrentHandAngle < 75 || CurrentHandAngle > 115)
		{
			return;
		}

		FHitResult FloorHit;
		FVector FloorCheckStart = GetActorLocation() + (GetActorForwardVector() * 6);
		FVector FloorCheckEnd = FloorCheckStart - FVector(0, 0, 10.f);
		FCollisionShape FloorCheckShape;
		FloorCheckShape.SetBox(FVector3f(12, 4, .5f));
		FCollisionQueryParams ColParams;
		ColParams.bTraceComplex = false;
		TSet<UActorComponent*> ActorComps = _CharacterAttachedTo->GetComponents();

		for (UActorComponent* AC : ActorComps)
		{
			if (UPrimitiveComponent* PC = Cast<UPrimitiveComponent>(AC))
			{
				ColParams.AddIgnoredComponent(PC);
			}
		}

		GetWorld()->SweepSingleByChannel(FloorHit, FloorCheckStart, FloorCheckEnd, GetActorQuat(),
			ECollisionChannel::ECC_Pawn, FloorCheckShape, ColParams);


		if (FloorHit.bBlockingHit && FloorHit.GetActor())
		{
			//DrawDebugBox(GetWorld(), FloorHit.Location, FVector(12, 4, .5f), GetActorQuat(), FColor::Red, false, 5.f);

			float FloorAngle = ExtraMaths::GetAngleOfTwoVectors(FloorHit.ImpactNormal, FVector::UpVector);

			if (FloorAngle < _MaxFloorAngleWeCanVault)
			{
				float AmountOfRayHits = 0;
				
				FVector ActorLoc = GetActorLocation();
				FVector ForwardDir = GetActorForwardVector();
				FVector RightDir = GetActorRightVector();
				FVector UpDir = GetActorUpVector();
				for (FVector StartLoc : _VaultingLineTracesLocations)
				{
					FHitResult Hit;
					FVector TraceStart = ActorLoc;
					TraceStart += StartLoc.X * ForwardDir;
					TraceStart += StartLoc.Y * RightDir;
					TraceStart += StartLoc.Z * UpDir;
					FVector TraceEnd = TraceStart + (FVector(0, 0, -1) * _VaultingLineTraceLength);
					
					if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_Pawn))
					{
						AmountOfRayHits++;
						//DrawDebugLine(GetWorld(), TraceStart, Hit.Location , FColor::Green, false, 5.f);
					}
					//else
						//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 5.f);

					
				}

				if (AmountOfRayHits >= _MinTraceHitsToAllowVaulting)
				{
					_Vaulting_Actor = FloorHit.GetActor();

					_ClimbingInfo._EGC_StartLocation = _Vaulting_Actor->GetActorLocation();
					_ClimbingGrabTransform = _ControllerSKM->GetComponentTransform();
					ChangeClimbingMode(EClimbingMode::ECM_Vaulting);
				}
			}
		}
	}
	else 
	{
		/*FVector CapLoc = _CharacterAttachedTo->GetCharacterCollisionLocation();
		FVector MCLoc = _MotionControllerComp->GetComponentLocation();
		FVector DirToHand = (MCLoc - CapLoc).GetSafeNormal();

		if (MCLoc.Z > CapLoc.Z || ExtraMaths::GetAngleOfTwoVectors(DirToHand, -_CharacterAttachedTo->GetActorForwardVector()) < 85)
			ChangeClimbingMode(false, EClimbingMode::ECM_None);*/
	}
}

void AVRHand::ChangeClimbingMode(EClimbingMode AttemptedClimbingMode)
{
	if (!_CharacterAttachedTo || !_MotionControllerComp ||!_PHC)
		return;


	switch (AttemptedClimbingMode)
	{
	case EClimbingMode::ECM_None:
		_Vaulting_Actor = NULL;
		_DynamicGrabbed_ActorGrabbed = NULL;
		_GrabbedEGC = NULL;
		_PHC->SetTargetType(ETargetType::ETT_SetObject);
		_ControllerSKM->SetAnimation(_DefaultHandPose);
		_ControllerSKM->SetSimulatePhysics(true);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), _SC_HandGrabbedClimbable, _ControllerSKM->GetComponentLocation());
		break;
	case EClimbingMode::ECM_GrabbedClimbing:
		_PHC->SetTargetType(ETargetType::ETT_SetLocationAndRotation);
		_ControllerSKM->SetAnimation(_EdgeLedgeGrabHandPose);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), _SC_HandGrabbedClimbable, GetActorLocation());
		_ControllerSKM->SetSimulatePhysics(false);
		break;
	case EClimbingMode::ECM_Vaulting:
		_PHC->SetTargetType(ETargetType::ETT_SetLocationAndRotation);
		_ControllerSKM->SetAnimation(_VaultingHandPose);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), _SC_HandGrabbedClimbable, GetActorLocation());
		_ControllerSKM->SetSimulatePhysics(false);
		break;
	default:
		break;
	}

	_ClimbingInfo._GrabbedLocation = GetActorLocation();//_MotionControllerComp->GetComponentLocation();
	_ClimbingInfo._HandQuat = GetActorQuat();
	_CharacterAttachedTo->HandGrabbedClimbingPoint(!GetIsRightHand(), GetActorQuat(), AttemptedClimbingMode);

}

void AVRHand::GripPressCheck()
{
	if (_ItemStorers.Num() > 0)
	{
		if (AVRItem* Item = _ItemStorers[0]->GetItemStored())
		{
			if (UItemGrabComponent* GrabComp = Item->GetGrabPoint())
			{
				if (GrabComp->GrabComponent(this))
				{
					_ComponentHeld = GrabComp;
					_ControllerSKM->SetSimulatePhysics(false);
					_ControllerSKM->SetVisibility(false);
				}
			}
		}
	}
	else if (_GrabCompArray.Num() > 0)
	{
		AttemptItemGrab();
	}
	else if (_AmmoStorageCompArray.Num() > 0)
	{
		if (AVRItem* Mag = _AmmoStorageCompArray[0]->GetMagazine(this))
		{
			UItemGrabComponent* GrabComp = Mag->GetGrabPoint();

			if (GrabComp)
			{
				if (GrabComp->GrabComponent(this))
				{				
					_ComponentHeld = GrabComp;
					_ControllerSKM->SetSimulatePhysics(false);
					//_ControllerSKM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					_ControllerSKM->SetVisibility(false);

					/*if (GetLocalRole() < ENetRole::ROLE_Authority)
						Server_GrabbedComponent(_ComponentHeld);*/
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(756, 10.f, FColor::Green, "AmmoGrab: GrabPoint UnSuccessful");
				}
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(756, 10.f, FColor::Green, "AmmoGrab: GrabPoint NULL");
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(756, 10.f, FColor::Green, "AmmoGrab: couldnt spawn Mag");

		}
	}
	else if (_EnvironmentGrabCompArray.Num() > 0)
	{
		if (!_EnvironmentGrabCompArray[0])
			return;

		_GrabbedEGC = _EnvironmentGrabCompArray[0];
		_GrabbedEGC->GrabComponent(this);
		_ClimbingInfo._EGC_StartLocation = _GrabbedEGC->GetComponentLocation();


		FVector NewGrabLocation = _EnvironmentGrabCompArray[0]->GetLocationOnZone(GetActorLocation());
		_ClimbingGrabTransform.SetLocation(NewGrabLocation - FVector(0, 0, _HandHeightClimbingGrabOffset));

		FQuat NewRotation(FRotator(90, 0, 0));

		float YawAngle = ExtraMaths::GetSignedAngleOfTwoVectors(NewRotation.GetUpVector(), -GetActorForwardVector(), -NewRotation.GetRightVector());
		FVector YawCrossP = FVector::CrossProduct(NewRotation.GetForwardVector(), GetActorForwardVector());
		FQuat YawQuatOffset = FQuat(FVector::ForwardVector, FMath::DegreesToRadians(YawAngle));
		NewRotation = NewRotation * YawQuatOffset;

		_ClimbingGrabTransform.SetRotation(NewRotation);


		ChangeClimbingMode(EClimbingMode::ECM_GrabbedClimbing);
	}
	else if (CheckDynamicEdgeGrap())
	{
		if (_DynamicGrabbed_ActorGrabbed)
		{
			_ClimbingInfo._EGC_StartLocation = _DynamicGrabbed_ActorGrabbed->GetActorLocation();

			ChangeClimbingMode(EClimbingMode::ECM_GrabbedClimbing);
		}
	}
	else if (!_DynamicGrabbed_ActorGrabbed && !_GrabbedEGC && !_Vaulting_Actor)
	{
		CheckForVaulting();
	}
}

void AVRHand::GrabSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (UItemGrabComponent* GC = Cast<UItemGrabComponent>(OtherComp))
	{
		_GrabCompArray.Add(GC);
	}
	else if (UEnvironmentGrabComponent* EGC = Cast<UEnvironmentGrabComponent>(OtherComp))
	{
		_EnvironmentGrabCompArray.Add(EGC);
	}
	else if (UAmmoStorageComponent* ASC = Cast<UAmmoStorageComponent>(OtherComp))
	{
		_AmmoStorageCompArray.Add(ASC);
		_ItemStorers;
	}
	else if (UItemStorer* IS = Cast<UItemStorer>(OtherComp))
	{
		_ItemStorers.Add(IS);
	}
}

void AVRHand::GrabSphereOverlapEnd(	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UItemGrabComponent* GC = Cast<UItemGrabComponent>(OtherComp))
	{
		_GrabCompArray.Remove(GC);
	}
	else if (UEnvironmentGrabComponent* EGC = Cast<UEnvironmentGrabComponent>(OtherComp))
	{
		_EnvironmentGrabCompArray.Remove(EGC);
	}
	else if (UAmmoStorageComponent* ASC = Cast<UAmmoStorageComponent>(OtherComp))
	{
		_AmmoStorageCompArray.Remove(ASC);
	}
	else if (UItemStorer* IS = Cast<UItemStorer>(OtherComp))
	{
		_ItemStorers.Remove(IS);
	}
}

void AVRHand::NF_Server_RequestCharacterOwner_Implementation()
{
	if (!_CharacterAttachedTo)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, "VRHand _CharacterAttachedTo is NULL!");
		return;
	}

	NF_Client_RequestCharacterOwner(_CharacterAttachedTo);
	ClientSideSetup(_CharacterAttachedTo->GetMotionController(GetIsRightHand()));
}

void AVRHand::NF_Client_RequestCharacterOwner_Implementation(AActor* CharacterOwner)
{
	GEngine->AddOnScreenDebugMessage(982, 15.f, FColor::Black, "Server sent the character owner");

	_CharacterAttachedTo = Cast<AVRCharacter>(CharacterOwner);

	if (_CharacterAttachedTo)
	{
		ClientSideSetup(_CharacterAttachedTo->GetMotionController(GetIsRightHand()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(982, 15.f, FColor::Black, "CharacterOwner is null!");
	}
}

void AVRHand::IsRightHand(bool bRightHand)
{
	_bRightHand = bRightHand;
	//_MotionControllerComp = ControllerToFollow;
	/*if (bRightHand)
	{
		if (_RightPhysicsHandSM)
		{
			_PhysicsHandSM = _RightPhysicsHandSM;
			_RightPhysicsHandSM->SetVisibility(true);
		}
		
		if (_LeftPhysicsHandSM)
			_LeftPhysicsHandSM->SetVisibility(false);

	}
	else
	{
		if (_LeftPhysicsHandSM)
		{
			_PhysicsHandSM = _LeftPhysicsHandSM;
			_LeftPhysicsHandSM->SetVisibility(true);
		}

		if (_RightPhysicsHandSM)
			_RightPhysicsHandSM->SetVisibility(false);
	}*/
}

FVector AVRHand::GetMotionControllerMoveForwardDir()
{
	if (!_MotionControllerComp || !_ControllerSKM)
		return FVector::ZeroVector;

	/*FVector StartLoc = _RootComp->GetComponentLocation();
	FVector EndLoc = _MotionControllerComp->GetComponentLocation();
	FVector Dir = EndLoc - StartLoc;
	Dir.Z = 0;
	ExtraMaths::CorrectNormalizedVector(Dir);

	return Dir;*/

	return FVector::ZeroVector;
}

void AVRHand::GripPressed()
{
	if (!_bDoneGrab)
	{
		_bDoneGrab = true;

		_GripCheckTimer = 1.0f;
	}

	/*if (_LeftPhysicsHandSM)
	{
		_PhysicsHandSM = _LeftPhysicsHandSM;
		_LeftPhysicsHandSM->SetVisibility(false);
	}

	if (_RightPhysicsHandSM)
	{
		_RightPhysicsHandSM->SetVisibility(false);
	}

	if (_ControllerSM)
		_ControllerSM->SetVisibility(true);*/
}

void AVRHand::GripReleased()
{
	_GripCheckTimer = 0;

	_bDoneGrab = false;

	if (_ComponentHeld)
	{
		SetActorLocation(_ComponentHeld->GetComponentLocation(), false, nullptr, ETeleportType::ResetPhysics);
		_ComponentHeld->ReleaseComponent(this);
		_ComponentHeld = NULL;
		_ControllerSKM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MoveHandOutsideOfHeldObject();
		_ControllerSKM->SetSimulatePhysics(true);
		_ControllerSKM->SetVisibility(true);
		Server_DroppedGrabbedComponent();	
	}

	if (_GrabbedEGC)
	{
		_GrabbedEGC->ReleaseComponent(this);	
		ChangeClimbingMode(EClimbingMode::ECM_None);

	}

	if (_DynamicGrabbed_ActorGrabbed || _Vaulting_Actor)
	{
		ChangeClimbingMode(EClimbingMode::ECM_None);
	}
}

void AVRHand::TopButtonPressed(bool bPressed)
{
	if (_ComponentHeld)
	{
		_ComponentHeld->TopButtonPressed(bPressed);
	}

	if (_GrabbedEGC)
	{
		_GrabbedEGC->TopButtonPressed(bPressed);
	}
}

void AVRHand::BottomButtonPressed(bool bPressed)
{
	if (_ComponentHeld)
	{
		_ComponentHeld->BottomButtonPressed(bPressed);
	}

	if (_GrabbedEGC)
	{
		_GrabbedEGC->BottomButtonPressed(bPressed);
	}
}

void AVRHand::TriggerPressed(float Value)
{
	if (_ComponentHeld)
	{
		_ComponentHeld->TriggerPressed(Value);
	}

	if (_GrabbedEGC)
	{
		_GrabbedEGC->TriggerPressed(Value);
	}
}

const FTransform AVRHand::GetTrackingHandTransform()
{
	if (!_MotionControllerComp)
		return FTransform();

	return _MotionControllerComp->GetComponentTransform();
}

void AVRHand::UpdateOwner(AController* PC)
{
	SetOwner(PC);

	if (!_PHC)
		return;

	/*_PHC->SetPhysicsObject(_ControllerSKM);
	_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
	_PHC->SetTargetObject(_MotionControllerComp);
//	_PHC->SetMatchTarget(true);
	_PHC->SetTargetType(ETargetType::ETT_SetObject);
	_PHC->SetPlayerControllerOwner(PC);*/
}

FTransform AVRHand::GetPhysicsObjectTransform()
{
	if(!_ControllerSKM)
		return FTransform();

	return _ControllerSKM->GetComponentTransform();
}

void AVRHand::SetCharacterAttachedTo(AVRCharacter* Character)
{
	_CharacterAttachedTo = Character;
	
}

FVector AVRHand::GetDesiredVelocity()
{
	if (_CharacterAttachedTo)
	{
		return _CharacterAttachedTo->GetDesiredVelocity();
	}

	return FVector::ZeroVector;
}

void AVRHand::NonVRFollow(USceneComponent* CompToFollow)
{
	if (_PHC)
	{
		_PHC->SetTargetObject(CompToFollow);
	}
}

AVRItem* AVRHand::GetHoldingItem()
{
	if(!_ComponentHeld)
		return nullptr;

	if (AVRItem* Item = Cast<AVRItem>(_ComponentHeld->GetAttachmentRootActor()))
	{
		return Item;
	}

	return nullptr;
}

void AVRHand::ClientSideSetup(UMotionControllerComponent* ControllerToFollow)
{
	_MotionControllerComp = ControllerToFollow;

	if (!_ControllerSKM)
	{
		GEngine->AddOnScreenDebugMessage(982, 15.f, FColor::Black, "ClientSideSetup: ControllerSKM is NULL!");
		return;
	}

	if (!_MotionControllerComp)
	{
		GEngine->AddOnScreenDebugMessage(982, 15.f, FColor::Black, "ClientSideSetup: _MotionControllerComp is NULL!");
		return;
	}

	GEngine->AddOnScreenDebugMessage(982, 15.f, FColor::Black, "ClientSideSetup");
	_PHC->SetPhysicsObject(_ControllerSKM);
	_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
	_PHC->SetTargetType(ETargetType::ETT_SetObject);
	_PHC->SetTargetObject(_MotionControllerComp);
	_PHC->SetPlayerControllerOwner(Cast<APlayerController>(GetOwner()));

}

void AVRHand::Client_ItemPickupInvalid_Implementation()
{
	GripReleased();
}

void AVRHand::Server_GrabbedComponent_Implementation(UItemGrabComponent* GrabComp)
{	
	if (!GrabComp)
		return;

	if (!GrabComp->CanGrabComponent())
	{
		Client_ItemPickupInvalid();
	}
	else
	{
		if (GrabComp->GrabComponent(this))
		{
			_ComponentHeld = GrabComp;
		}
		else
		{
			Client_ItemPickupInvalid();
		}
	}
}

void AVRHand::Server_DroppedGrabbedComponent_Implementation()
{
	if (!_ComponentHeld)
	{
		//Client_ItemPickupInvalid();
		return;
	}
	
	_ComponentHeld->ReleaseComponent(this);
	_ComponentHeld = NULL;
}

void AVRHand::AttemptItemGrab()
{
	if (!_ControllerSKM || _ComponentHeld)
		return;

	UItemGrabComponent* GrabComp = FindClosestGrabComponent();

	if (GrabComp)
	{
		if (GrabComp->GrabComponent(this))
		{
			_ComponentHeld = GrabComp;
			_ControllerSKM->SetSimulatePhysics(false);
			_ControllerSKM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			_ControllerSKM->SetVisibility(false);

			/*if(GetLocalRole() < ENetRole::ROLE_Authority)
				Server_GrabbedComponent(_ComponentHeld);*/
		}
	}
}

UItemGrabComponent* AVRHand::FindClosestGrabComponent()
{
	if (!_GrabSphere || _GrabCompArray.Num() <= 0)
		return NULL;

	float ShortDist = 99999;
	UItemGrabComponent* GrabComp = NULL;

	for (UItemGrabComponent* GC : _GrabCompArray)
	{
		if (GC->CanGrabComponent())
		{
			float Dist = FVector::DistSquared(_GrabSphere->GetComponentLocation(), GC->GetComponentLocation());
			if (Dist < ShortDist)
			{
				ShortDist = Dist;
				GrabComp = GC;
				//GEngine->AddOnScreenDebugMessage(2, 1.0, FColor::Red, "Short Dist: " + FString::SanitizeFloat(ShortDist));
			}
		}
	}		

	return GrabComp;
}