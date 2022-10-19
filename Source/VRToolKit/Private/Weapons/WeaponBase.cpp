// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"
#include "Items/ItemGrabComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Player/VRHand.h"
#include "MotionControllerComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//setup skeletal mesh
	/*_WeaponSKM = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh");
	RootComponent = _WeaponSKM;*/

	//setup hand grab pose
	_HandGrabPose = CreateDefaultSubobject<USkeletalMeshComponent>("HandGrabPose");
	_HandGrabPose->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (_HandGrabPose)
		_HandGrabPose->SetVisibility(false);
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	EditWeaponOffset(DeltaTime);
}

void AWeaponBase::EditWeaponOffset(float DeltaTime)
{
	if (_MainGrabComponent && _MainGrabComponent->GetHand())
	{
		FQuat HandRot = _MainGrabComponent->GetHand()->GetTrackingHandTransform().GetRotation();
		FQuat Offset = FQuat(FVector::CrossProduct(HandRot.GetForwardVector(), HandRot.GetUpVector()), -45);
		_PHC->SetTargetRotationOffset(Offset);
	}
}

void AWeaponBase::MainGrabPointGrabbed(AVRHand* Hand)
{
	Super::MainGrabPointGrabbed(Hand);

	if (_HandGrabPose)
		_HandGrabPose->SetVisibility(true);

}

void AWeaponBase::MainGrabPointReleased(AVRHand* Hand)
{
	Super::MainGrabPointReleased(Hand);

	if (_HandGrabPose)
		_HandGrabPose->SetVisibility(false);
}

