// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/PhysicsRotationComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Utility/ExtraMaths.h"

// Sets default values for this component's properties
UPhysicsRotationComponent::UPhysicsRotationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	_OnCalculateCustomPhysics.BindUObject(this, &UPhysicsRotationComponent::CustomPhysics);
	SetTickGroup(ETickingGroup::TG_PostPhysics);
	// ...
}


// Called when the game starts
void UPhysicsRotationComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}


// Called every frame
void UPhysicsRotationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!_PhysicsComp)
		return; 

	if (_PhysicsComp->GetBodyInstance())
	{
		//_PhysicsComp->GetBodyInstance()->AddCustomPhysics(_OnCalculateCustomPhysics);
	}


	// ...
}

void UPhysicsRotationComponent::SetBaseRotationToLockTo(FQuat BaseRotation)
{
	_BaseRotation = BaseRotation;
}

void UPhysicsRotationComponent::SetMinMaxAngle(float MinAngle, float MaxAngle, bool bLockToAngle)
{
	_MinAngle = MinAngle;
	_MaxAngle = MaxAngle;
	_bLockToAngle = bLockToAngle;
}

void UPhysicsRotationComponent::SetLockToAngle(bool bLockToAngle)
{
	_bLockToAngle = bLockToAngle;
}

void UPhysicsRotationComponent::SetLockedAxis(FVector LockedAxis, bool bLockRotationToAxis)
{
	_LockedAxis = LockedAxis;
	_bLockRotationToAxis = bLockRotationToAxis;
}

void UPhysicsRotationComponent::SetLockRototationToAxis(bool bLockRotationToAxis)
{
	_bLockRotationToAxis = bLockRotationToAxis;
}

void UPhysicsRotationComponent::SetPhysicsComponentToMove(UPrimitiveComponent* PhysicsComp)
{
	_PhysicsComp = PhysicsComp;
}

void UPhysicsRotationComponent::SetGotoRotation(FQuat GotoRotation, bool bGotoRotation)
{
	_GotoRotation = GotoRotation;
	_bGotoRotation = bGotoRotation;
}

void UPhysicsRotationComponent::SetCanGotoRotation(bool bGotoRotation)
{
	_bGotoRotation = bGotoRotation;
}

void UPhysicsRotationComponent::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	FBodyInstance* BI = _PhysicsComp->GetBodyInstance();
	FTransform BI_Transform = BI->GetUnrealWorldTransform_AssumesLocked();
	float Speed = BI->GetUnrealWorldAngularVelocityInRadians_AssumesLocked().Size();
	FQuat CurrentRot = BI_Transform.GetRotation();

	float Angle = ExtraMaths::GetAngleOfTwoVectors(_BaseRotation.GetUpVector(), CurrentRot.GetUpVector());
	bool AllowedAngle = true;

	if (ExtraMaths::GetAngleOfTwoVectors(_BaseRotation.GetForwardVector(), CurrentRot.GetUpVector()) > 90)
		Angle = -Angle;

	if (Angle > _MaxAngle)
		AllowedAngle = false;

	if (_bLockToAngle)
		Angle = FMath::Clamp(Angle, _MinAngle, _MaxAngle);

	FQuat RotOffset = FQuat(FVector::CrossProduct(_BaseRotation.GetUpVector(), _BaseRotation.GetForwardVector()), FMath::DegreesToRadians(Angle));

	if (_bLockRotationToAxis)
		RotOffset = FQuat(_LockedAxis, FMath::DegreesToRadians(Angle));

	FQuat DesiredRot = RotOffset * _BaseRotation;
	FTransform NewT = BI_Transform;
	NewT.SetRotation(DesiredRot);


	BI->SetBodyTransform(NewT, ETeleportType::TeleportPhysics);

	/*FVector T = _QuatPID.GetTorque(DeltaTime, CurrentRot, DesiredRot,
		FVector::RadiansToDegrees(BI->GetUnrealWorldAngularVelocityInRadians_AssumesLocked()),
		BI->GetBodyInertiaTensor(), BI_Transform);*/

	//BI->AddTorqueInRadians(FVector::DegreesToRadians(T));

	FVector AVel = FVector::ZeroVector;
	if (AllowedAngle)
		AVel = FVector::CrossProduct(_BaseRotation.GetUpVector(), _BaseRotation.GetForwardVector());
	else
		AVel = FVector::ZeroVector;
	
	FVector AxisVel = BI->GetUnrealWorldAngularVelocityInRadians_AssumesLocked();
	AxisVel.Normalize();

	if (ExtraMaths::GetAngleOfTwoVectors(_BaseRotation.GetForwardVector(), AxisVel) < 90)
		AVel *= Speed;
	else
		AVel *= -Speed;

	BI->SetAngularVelocityInRadians(AVel, false);
	//BI->Velocity
}

