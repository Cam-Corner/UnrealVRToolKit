// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Online/NetworkHelpers.h"
#include "ActorComponents/PhysicsHandlerComponentAsync.h"
#include "PBDRigidsSolver.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "Online/PhysicsReplicationComponent.h"


// Sets default values for this component's properties
UPhysicsHandlerComponent::UPhysicsHandlerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//_OnCalculateCustomPhysics.BindUObject(this, &UPhysicsHandlerComponent::CustomPhysics);
	SetIsReplicated(true);
	// ...

	//_LocPD._Frequency = 6.0f;
	//_LocPD._Dampening = 1.0f;
	//_RotPD._Frequency = 6.0f;
	//_RotPD._Dampening = 1.0;

	_TargetRotationOffset = FQuat(FRotator(0, 0, 0));

	PrePhysicsTickFunction.bCanEverTick = true;
	PrePhysicsTickFunction.bStartWithTickEnabled = true;
	PrePhysicsTickFunction.SetTickFunctionEnable(true);
	PrePhysicsTickFunction.TickGroup = TG_PrePhysics;

	PostPhysicsTickFunction.bCanEverTick = true;
	PostPhysicsTickFunction.bStartWithTickEnabled = true;
	PostPhysicsTickFunction.SetTickFunctionEnable(true);
	PostPhysicsTickFunction.TickGroup = TG_PostPhysics;
}


// Called when the game starts
void UPhysicsHandlerComponent::BeginPlay()
{
	Super::BeginPlay();
	//_DefaultTickTimer = 1 / NETWORKHELPERS_SERVER_TICKRATE;
	//_TickTimer = _DefaultTickTimer;
	// ...

	RegisterAsyncCallback();
}

void UPhysicsHandlerComponent::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	if (bRegister)
	{
		if (SetupActorComponentTickFunction(&PrePhysicsTickFunction))
		{
			PrePhysicsTickFunction.Target = this;
			PrePhysicsTickFunction.AddPrerequisite(this, this->PrimaryComponentTick);
		}

		if (SetupActorComponentTickFunction(&PostPhysicsTickFunction))
		{
			PostPhysicsTickFunction.Target = this;
			PostPhysicsTickFunction.AddPrerequisite(this, this->PrimaryComponentTick);
		}
	}
	else
	{
		if (PrePhysicsTickFunction.IsTickFunctionRegistered())
		{
			PrePhysicsTickFunction.UnRegisterTickFunction();
		}

		if (PostPhysicsTickFunction.IsTickFunctionRegistered())
		{
			PostPhysicsTickFunction.UnRegisterTickFunction();
		}
	}

}

void UPhysicsHandlerComponent::PrePhysicsTickComponent(float DeltaTime, FPhysicsHandlerPrePhysicsTickFunction& ThisTickFunction)
{
	BuildAsyncInput();
}

void UPhysicsHandlerComponent::PostPhysicsTickComponent(float DeltaTime, FPhysicsHandlerPostPhysicsTickFunction& ThisTickFunction)
{

}

// Called every frame
void UPhysicsHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*if (_CheckTimer <= 0)
	{
		_CheckTimer = 1;
		GEngine->AddOnScreenDebugMessage(12341, 1.5f, FColor::Yellow, "StepsRan: " + FString::SanitizeFloat(_StepsRan));
		_StepsRan = 0;
	}

	if (!_PhysicsObject || !_bComponentEnabled)
		return;

	if (_PhysicsObject->GetBodyInstance())
	{
		_PhysicsObject->GetBodyInstance()->AddCustomPhysics(_OnCalculateCustomPhysics);
	}

	_CheckTimer -= DeltaTime;*/

	_TimeBeforeSending -= DeltaTime;
	

	//if (GetOwnerRole() < ENetRole::ROLE_AutonomousProxy)
	{
		AActor* Owner = Cast<AActor>(GetOwner());
		if (_TimeBeforeSending <= 0 && _PhysicsObject)
		{
			if (GetOwnerRole() >= ENetRole::ROLE_Authority)
			{
				GEngine->AddOnScreenDebugMessage(10, 1, FColor::Blue, "Owner = " + GetOwner()->GetFName().ToString());

				if (Owner && Owner->GetOwner())
					GEngine->AddOnScreenDebugMessage(11, 1, FColor::Blue, "PC Owner = " + Owner->GetOwner()->GetFName().ToString());
				else
					GEngine->AddOnScreenDebugMessage(11, 1, FColor::Blue, "PC Owner = No Owner!");
			}

			if (Owner && Owner->GetOwner() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				FPhysicsNetSnapShot SnapShot;
				SnapShot._Location = Owner->GetActorLocation();//_PhysicsObject->GetComponentLocation();
				SnapShot._Rotation = _PhysicsObject->GetComponentQuat();
				SnapShot._LinearVelocity = _PhysicsObject->GetPhysicsLinearVelocity();
				SnapShot._AngularVelocity = _PhysicsObject->GetPhysicsAngularVelocityInRadians();
				NF_Server_SendPhysicsSnapShot(SnapShot);
				GEngine->AddOnScreenDebugMessage(12, 1, FColor::Blue, "SentPhysicsState");
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(12, 1, FColor::Blue, "Not Owner So didnt send physics state!");
			}

			_TimeBeforeSending = 1 / 60;
		}
		else if (!_PhysicsObject)
		{
			if (GetOwnerRole() >= ENetRole::ROLE_Authority && Owner && Owner->GetOwner() != UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				GEngine->AddOnScreenDebugMessage(100, 1, FColor::Red, "Server _PhysicsObject is NULL!");
			}
			else if(Owner && Owner->GetOwner() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				GEngine->AddOnScreenDebugMessage(101, 1, FColor::Red, GetName() + " Client _PhysicsObject is NULL!");
			}
		}
	}
}

void UPhysicsHandlerComponent::SetMatchTargetAuthorityType(EAuthorityType MatchTargetAuthorityType)
{
	_MatchTargetAuthorityType = MatchTargetAuthorityType;
}

void UPhysicsHandlerComponent::SetTargetType(ETargetType TargetType)
{
	_MatchTargetType = TargetType;
}

void UPhysicsHandlerComponent::SetPhysicsObject(UPrimitiveComponent* PComp)
{
	_PhysicsObject = PComp;
}

void UPhysicsHandlerComponent::SetTargetObject(USceneComponent* Comp)
{
	_TargetObject = Comp;
}

void UPhysicsHandlerComponent::SetTargetLocation(FVector Location)
{
	_TransformToMatch.SetLocation(Location);
}

void UPhysicsHandlerComponent::SetTargetRotation(FQuat Rotation)
{
	_TransformToMatch.SetRotation(Rotation);
}

void UPhysicsHandlerComponent::SetTargetLocationAndRotation(FVector Location, FQuat Rotation)
{
	SetTargetLocation(Location);
	SetTargetRotation(Rotation);
}

void UPhysicsHandlerComponent::SetTargetTransform(FTransform Transform)
{
	SetTargetLocation(Transform.GetLocation());
	SetTargetRotation(Transform.GetRotation());
}

/*void UPhysicsHandlerComponent::SetMatchTarget(bool bMatchTarget)
{
	_bMatchTarget = bMatchTarget;
}*/

void UPhysicsHandlerComponent::SetPlayerControllerOwner(AController* PC)
{
	_PCOwner = PC;
}

void UPhysicsHandlerComponent::EnableComponent(bool bEnabled)
{
	_bComponentEnabled = bEnabled;
}

void UPhysicsHandlerComponent::SetTargetLocationOffset(FVector Offset)
{
	_TargetLocationOffset = Offset;
}

void UPhysicsHandlerComponent::SetTargetRotationOffset(FQuat Offset)
{
	_TargetRotationOffset = Offset;

}

/*void UPhysicsHandlerComponent::UpdateComponent(float DeltaTime)
{
	if (!_PhysicsObject || !_bComponentEnabled)
		return;

	if (_PhysicsObject->GetBodyInstance())
	{
		_PhysicsObject->GetBodyInstance()->AddCustomPhysics(_OnCalculateCustomPhysics);
	}
}*/


void UPhysicsHandlerComponent::RegisterAsyncCallback()
{
	if (UWorld* World = GetWorld())
	{
		if (FPhysScene* FPhysScene = World->GetPhysicsScene())
		{
#if WITH_CHAOS
			_AsyncCallback = FPhysScene->GetSolver()->CreateAndRegisterSimCallbackObject_External<PhysicsHandlerComponentAsync>();
#endif
		}

	}
}

void UPhysicsHandlerComponent::BuildAsyncInput()
{
	if (IsAsyncCallbackRegistered())
	{
		FPhysicsHandlerAsyncInput* AsyncInput = _AsyncCallback->GetProducerInputData_External();

		AsyncInput->_PhysicsHandler = this;
		//Initialize stuff here and do things before the physics tick ready for it to be simulated

	}
}

void UPhysicsHandlerComponent::ProccessAsyncOuput()
{
	if (IsAsyncCallbackRegistered())
	{
		auto AsyncInput = _AsyncCallback->PopOutputData_External();

		//Access any output data after the simulation tick has been run

	}
}

void UPhysicsHandlerComponent::AsyncTick(float DeltaTime)
{
	/*_AsyncTimer -= DeltaTime;
	_AsyncTicksRan++;

	if (_AsyncTimer <= 0)
	{
		_AsyncTimer = 1;
		_AsyncTicksRan = 0;	
	}*/
	

	if (_MatchTargetType == ETargetType::ETT_NoTarget)
		APT_DefaultPhysics(DeltaTime);
	else
		APT_MatchTarget(DeltaTime);
	
}

/*void UPhysicsHandlerComponent::UpdateToNextPhysicsFrame()
{
	if (_TickTimer > 0 || !_PhysicsObject)
		return;

	FBodyInstance* PO_BI = _PhysicsObject->GetBodyInstance();

	if (!PO_BI)
		return;

	FPhysicsFrame PF = GetNewPhysicsFrame();

	FTransform T;
	T.SetLocation(PF._Location);
	T.SetRotation(PF._Rotation);
	PO_BI->SetBodyTransform(T, ETeleportType::ResetPhysics);

	PO_BI->SetLinearVelocity(PF._Linear_Velocity, false);
	PO_BI->SetAngularVelocityInRadians(PF._Angular_Velocity, false);

	_Known_Goto_Location = PF._Goto_Location;
	_Known_Goto_Rotation = PF._Goto_Rotation;
}*/

/*void UPhysicsHandlerComponent::PhysicsTick_DefaultPhysics(float SubsetDeltaTime)
{
	if (_PCOwner)
	{
		if (_PCOwner->GetLocalRole() >= ENetRole::ROLE_Authority)
		{
			FBodyInstance* PO_BI = _PhysicsObject->GetBodyInstance();

			if (!PO_BI)
				return;

			if (_TickTimer <= 0)
			{
				FTransform WorldT = PO_BI->GetUnrealWorldTransform_AssumesLocked();

				FPhysicsFrame PF;
				PF._Location = WorldT.GetLocation();
				PF._Rotation = WorldT.GetRotation();
				PF._Linear_Velocity = PO_BI->GetUnrealWorldVelocity();
				PF._Angular_Velocity = PO_BI->GetUnrealWorldAngularVelocityInRadians();

				//If both velocitys are 0 then their is no need to send a physics update (helps to save bandwith when the object isnt moving)
				if (PF._Linear_Velocity != FVector::ZeroVector || PF._Angular_Velocity != FVector::ZeroVector)
					SendPhysicsTick(PF);
			}
		}
		else
		{
			//UpdateToNextPhysicsFrame();
		}
	}
	else
	{
		//UpdateToNextPhysicsFrame();
	}
}*/

/*void UPhysicsHandlerComponent::PhysicsTick_MatchTarget(float SubsetDeltaTime, FBodyInstance* BodyInstance)
{
	if (!_PhysicsObject || !_TargetObject)
		return;

	FBodyInstance* PO_BI = BodyInstance;

	EAuthorityType EAT = _MatchTargetAuthorityType;

	if (!_PCOwner || !PO_BI)
		return;

	ENetRole ENR = _PCOwner->GetLocalRole();
	
	FTransform WorldT = PO_BI->GetUnrealWorldTransform_AssumesLocked();//GetSubstepComponentWorldTransform(_PhysicsObject, PO_BI);// 

	FTransform TTransform;	

	if ((EAT == EAuthorityType::EAT_Client && _PCOwner == UGameplayStatics::GetPlayerController(GetWorld(), 0)) 
		|| (EAT != EAuthorityType::EAT_Client && ENR >= ENetRole::ROLE_Authority))
	{
		TTransform = _TargetObject->GetComponentTransform();

		/*if (FVector::DistSquared(WorldT.GetLocation(), TTransform.GetLocation()) > 5000)
		{
			PO_BI->SetBodyTransform(TTransform, ETeleportType::ResetPhysics);
			PO_BI->SetAngularVelocityInRadians(FVector::ZeroVector, false);
			PO_BI->SetLinearVelocity(FVector::ZeroVector, false);
		}

		if (_TickTimer <= 0)
		{
			FPhysicsFrame PF;
			PF._Location = WorldT.GetLocation();
			PF._Rotation = WorldT.GetRotation();
			PF._Linear_Velocity = PO_BI->GetUnrealWorldVelocity();
			PF._Angular_Velocity = PO_BI->GetUnrealWorldAngularVelocityInRadians();
			PF._Goto_Location = TTransform.GetLocation();
			PF._Goto_Rotation = TTransform.GetRotation();

			//If both velocitys are 0 then their is no need to send a physics update (helps to save bandwith when the object isnt moving)
			if (PF._Linear_Velocity != FVector::ZeroVector || PF._Angular_Velocity != FVector::ZeroVector)
				SendPhysicsTick(PF);
		}
	}
	else 
	{
		//UpdateToNextPhysicsFrame();

		//TTransform.SetLocation(_Known_Goto_Location);
		//TTransform.SetRotation(_Known_Goto_Rotation);
	}

	{
		//Linear Force
		FVector From = WorldT.GetLocation();
		FVector To = TTransform.GetLocation() + _TargetLocationOffset;

		if (_LocLastStep != From)
		{
			//_LocPD.Reset();
		}


		FVector Force = FVector::ZeroVector;//_LocPD.GetForce(SubsetDeltaTime, From, To, PO_BI->GetUnrealWorldVelocity(), FVector(0, 0, 0));
		//Force = Force.GetClampedToMaxSize(_MaxLinearVelocity);
		PO_BI->AddForce(Force * PO_BI->GetBodyMass(), false);
		_LocLastStep = From;
	}

	{
		FQuat From = WorldT.GetRotation();
		FQuat To = TTransform.GetRotation();
		//FQuat Offset = FQuat(FVector::CrossProduct(To.GetForwardVector(), To.GetUpVector()), -45);
		To *= _TargetRotationOffset;

		FVector Torque = _RotPD.GetTorque(SubsetDeltaTime, From,
			To, PO_BI->GetUnrealWorldAngularVelocityInRadians(),
			FVector(0, 0, 0), WorldT);

		{
			const FVector LocalInertiaTensor = PO_BI->GetBodyInertiaTensor();
			const FVector InputVectorLocal = WorldT.InverseTransformVectorNoScale(Torque);
			const FVector LocalScaled = InputVectorLocal * LocalInertiaTensor;
			const FVector WorldScaled = WorldT.TransformVectorNoScale(LocalScaled);
			Torque = WorldScaled;
		}

		
		PO_BI->AddTorqueInRadians(Torque, false);
	}

}*/

/*void UPhysicsHandlerComponent::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	_StepsRan += 1;
	_TickTimer -= DeltaTime;

	if (!_bMatchTarget)
		PhysicsTick_DefaultPhysics(DeltaTime);
	else
		PhysicsTick_MatchTarget(DeltaTime, BodyInstance);


	if (_TickTimer <= 0)
		_TickTimer = _DefaultTickTimer;
}*/

FTransform UPhysicsHandlerComponent::GetSubstepComponentWorldTransform(const USceneComponent* SceneComp, FBodyInstance* BI)
{
	if (!BI)
		return FTransform();

	FTransform FinalT = FTransform::Identity;

	if (SceneComp->GetAttachSocketName() == NAME_None)
	{
		if (USkeletalMeshComponent* SK = Cast<USkeletalMeshComponent>(BI->OwnerComponent.Get()))
		{
			const auto RootBoneName = SK->GetBoneName(0);
			const auto BIRefPose = GetBoneRefTransformInComponentSpace(SK, RootBoneName);
			return BIRefPose.Inverse() * BI->GetUnrealWorldTransform();
		}
	}

	return BI->GetUnrealWorldTransform();
}

FTransform UPhysicsHandlerComponent::GetBoneRefTransformInComponentSpace(const USkeletalMeshComponent* SK, const FName& BoneName)
{
	FTransform FinalT = FTransform::Identity;
	GetBoneTrasnformRecursive(FinalT, SK, BoneName);
	return FinalT;
}

void UPhysicsHandlerComponent::GetBoneTrasnformRecursive(FTransform& Transform, const USkeletalMeshComponent* SK, const FName& BoneName)
{
	if (!SK)
		return;

	Transform *= FTransform(SK->SkeletalMesh->GetRefPoseMatrix(SK->GetBoneIndex(BoneName)));
	const FName ParentBone = SK->GetParentBone(BoneName);

	if (ParentBone != NAME_None)
	{
		GetBoneTrasnformRecursive(Transform, SK, ParentBone);
	}
}

void UPhysicsHandlerComponent::APT_AddForce(UPrimitiveComponent* Comp, FVector Force, bool bHardSetVelocity)
{
	if (!Comp)
		return;

	if (FBodyInstance* BI = Comp->GetBodyInstance())
	{
		if (auto Handle = BI->ActorHandle)
		{
			if (Chaos::FRigidBodyHandle_Internal* RH = Handle->GetPhysicsThreadAPI())
			{
				Comp->WakeAllRigidBodies();

				if (bHardSetVelocity)
					BI->SetLinearVelocity(Force, false);
				else
					RH->AddForce(Force, false);
			}
		}
	}
}

void UPhysicsHandlerComponent::APT_AddTorque(UPrimitiveComponent* Comp, FVector Torque, bool bHardSetVelocity)
{
	if (!Comp)
		return;

	if (FBodyInstance* BI = Comp->GetBodyInstance())
	{
		if (auto Handle = BI->ActorHandle)
		{
			if (Chaos::FRigidBodyHandle_Internal* RH = Handle->GetPhysicsThreadAPI())
			{
				Comp->WakeAllRigidBodies();

				if (bHardSetVelocity)
					BI->SetAngularVelocityInRadians(Torque, false);
				else
					RH->AddTorque(Torque, false);
			}
		}
	}

}

void UPhysicsHandlerComponent::APT_MatchTarget(float DeltaTime)
{
	if (!_PhysicsObject || (!_TargetObject && _MatchTargetType == ETargetType::ETT_SetObject))
		return;

	FBodyInstance* PO_BI = _PhysicsObject->GetBodyInstance();

	if (!PO_BI)
		return;
	
	FTransform WorldT = PO_BI->GetUnrealWorldTransform_AssumesLocked();
	FTransform ToT = FTransform();// _TargetObject->GetComponentTransform();

	switch (_MatchTargetType)
	{
	case ETargetType::ETT_SetLocationAndRotation:
		ToT = _TransformToMatch;
		break;
	case ETargetType::ETT_SetObject:
		ToT = _TargetObject->GetComponentTransform();
		break;
	default:
		break;
	}

	{
		//Linear Force
		FVector From = WorldT.GetLocation();
		FVector To = ToT.GetLocation() + _TargetLocationOffset;		
		FVector Force = _LocPD.GetForce(DeltaTime, From, To, PO_BI->GetUnrealWorldVelocity(), _DesiredVelocity);
		APT_AddForce(_PhysicsObject , Force * PO_BI->GetBodyMass(), _bHardSetLinearVelocity);
		//GEngine->AddOnScreenDebugMessage(453, 1.f, FColor::Red, "PHC Desired Velocity: " + _DesiredVelocity.ToString());
	}

	{
		FQuat From = WorldT.GetRotation();
		FQuat To = ToT.GetRotation();
		FQuat Offset = FQuat(FVector::CrossProduct(To.GetForwardVector(), To.GetUpVector()), -45);
		To *= _TargetRotationOffset;

		FVector Torque = _RotPD.GetTorque(DeltaTime, From,
			To, PO_BI->GetUnrealWorldAngularVelocityInRadians(),
			FVector(0, 0, 0), WorldT);

		{
			const FVector LocalInertiaTensor = PO_BI->GetBodyInertiaTensor();
			const FVector InputVectorLocal = WorldT.InverseTransformVectorNoScale(Torque);
			const FVector LocalScaled = InputVectorLocal * LocalInertiaTensor;
			const FVector WorldScaled = WorldT.TransformVectorNoScale(LocalScaled);
			Torque = WorldScaled;
		}

		APT_AddTorque(_PhysicsObject, Torque, _bHardSetAngularVelocity);
	}
}

void UPhysicsHandlerComponent::APT_DefaultPhysics(float DeltaTime)
{

}

void UPhysicsHandlerComponent::NF_Server_SendPhysicsSnapShot_Implementation(const FPhysicsNetSnapShot& SnapShot)
{
	NF_NetMulticast_SendPhysicsSnapShot(SnapShot);
}

void UPhysicsHandlerComponent::NF_NetMulticast_SendPhysicsSnapShot_Implementation(const FPhysicsNetSnapShot& SnapShot)
{
	AActor* Owner = Cast<AActor>(GetOwner());
	if ((Owner && Owner->GetOwner() != UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{		
		GEngine->AddOnScreenDebugMessage(57, 1, FColor::Black , "Recieved Updated Physics State From Server!");

		FString LocationMessage = "SnapShot Info Location: " + SnapShot._Location.ToString();
		FString RotationMessage = "SnapShot Info Rotation: " + SnapShot._Rotation.ToString();
		FString LinearVelocityMessage = "SnapShot Info Linear Velocity: " + SnapShot._LinearVelocity.ToString();
		FString AngularVelocityMessage = "SnapShot Info Angular Velocity: " + SnapShot._AngularVelocity.ToString();

		GEngine->AddOnScreenDebugMessage(1, 1, FColor::Black, LocationMessage);
		GEngine->AddOnScreenDebugMessage(2, 1, FColor::Black, RotationMessage);
		GEngine->AddOnScreenDebugMessage(3, 1, FColor::Black, LinearVelocityMessage);
		GEngine->AddOnScreenDebugMessage(4, 1, FColor::Black, AngularVelocityMessage);

		if (!_PhysicsObject)
			return;

		_PhysicsObject->SetWorldLocation(SnapShot._Location, false, NULL, ETeleportType::TeleportPhysics);
		_PhysicsObject->SetWorldRotation(SnapShot._Rotation, false, NULL, ETeleportType::TeleportPhysics);
		_PhysicsObject->SetPhysicsLinearVelocity(SnapShot._LinearVelocity);
		_PhysicsObject->SetPhysicsAngularVelocityInRadians(SnapShot._AngularVelocity);
	}
}

void FPhysicsHandlerPrePhysicsTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	FActorComponentTickFunction::ExecuteTickHelper(Target, false, DeltaTime, TickType, [this](float DilatedTime)
		{
			Target->PrePhysicsTickComponent(DilatedTime, *this);
		});
}

FString FPhysicsHandlerPrePhysicsTickFunction::DiagnosticMessage()
{
	return Target->GetFullName() + TEXT("[UPhysicsHandlerComponent::PrePhysicsTick]");
}

FName FPhysicsHandlerPrePhysicsTickFunction::DiagnosticContext(bool bDetailed)
{
	if (bDetailed)
	{
		return FName(*FString::Printf(TEXT("PhysicsHandlerComponentPrePhysicsTick/%s"), *GetFullNameSafe(Target)));
	}

	return FName(TEXT("PhysicsHandlerComponentPrePhysicsTick"));
}

void FPhysicsHandlerPostPhysicsTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	FActorComponentTickFunction::ExecuteTickHelper(Target, false, DeltaTime, TickType, [this](float DilatedTime)
		{
			Target->PostPhysicsTickComponent(DilatedTime, *this);
		});
}

FString FPhysicsHandlerPostPhysicsTickFunction::DiagnosticMessage()
{
	return Target->GetFullName() + TEXT("[UPhysicsHandlerComponent::PrePhysicsTick]");
}

FName FPhysicsHandlerPostPhysicsTickFunction::DiagnosticContext(bool bDetailed)
{
	if (bDetailed)
	{
		return FName(*FString::Printf(TEXT("PhysicsHandlerComponentPostPhysicsTick/%s"), *GetFullNameSafe(Target)));
	}

	return FName(TEXT("PhysicsHandlerComponentPostPhysicsTick"));
}


