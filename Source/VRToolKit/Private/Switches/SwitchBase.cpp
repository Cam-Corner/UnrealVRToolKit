// Fill out your copyright notice in the Description page of Project Settings.


#include "Switches/SwitchBase.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/PhysicsRotationComponent.h"
#include "Items/ItemGrabComponent.h"
#include "Player/VRHand.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASwitchBase::ASwitchBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_StaticSwitchBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticSwitchBaseMesh");
	RootComponent = _StaticSwitchBaseMesh;
	_StaticSwitchBaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	_MovingSwitchPartMesh = CreateDefaultSubobject<UStaticMeshComponent>("MovingSwitchPartMesh");
	_MovingSwitchPartMesh->SetupAttachment(_StaticSwitchBaseMesh);
	_MovingSwitchPartMesh->SetSimulatePhysics(false);
	_MovingSwitchPartMesh->SetMassScale(NAME_None, 1);
	_MovingSwitchPartMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	_MovingSwitchPartMesh->SetEnableGravity(false);
	_MovingSwitchPartMesh->SetMassOverrideInKg(NAME_None, 100, true);
	_MovingSwitchPartMesh->bReplicatePhysicsToAutonomousProxy = false;
	_MovingSwitchPartMesh->OnComponentHit.AddDynamic(this, &ASwitchBase::OnHit);

	_GrabComp = CreateDefaultSubobject<UItemGrabComponent>("GrabComp");
	_GrabComp->SetupAttachment(_MovingSwitchPartMesh);

	_GrabComp->_ComponentGrabbed.AddDynamic(this, &ASwitchBase::ComponentGrabbed);
	_GrabComp->_ComponentReleased.AddDynamic(this, &ASwitchBase::ComponentReleased);
}

// Called when the game starts or when spawned
void ASwitchBase::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ASwitchBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (_bHandHoldingSwitch && _HandGrabbed)
	{
		HoldingSwitch(DeltaTime, _HandGrabbed->GetTrackingHandTransform().GetLocation(), _HandGrabbed->GetTrackingHandTransform().GetRotation());
	}
	else
	{
		DefaultAction(DeltaTime);
	}
}

void ASwitchBase::HoldingSwitch(float DeltaTime, FVector HandLocation, FQuat HandRotation)
{
}

void ASwitchBase::DefaultAction(float DeltaTime)
{
}

void ASwitchBase::HitARigidBody(const FHitResult& Hit)
{

}

void ASwitchBase::SwitchEnabled(bool bEnabled)
{
	_SwitchEnabled.Broadcast(bEnabled);
}

void ASwitchBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherComp && OtherComp->IsSimulatingPhysics() && Hit.bBlockingHit)
	{
		HitARigidBody(Hit);
	}

}

void ASwitchBase::ComponentGrabbed(AVRHand* HandGrabbed)
{
	_HandGrabbed = HandGrabbed;
	_bHandHoldingSwitch = true;
}

void ASwitchBase::ComponentReleased(AVRHand* Hand)
{
	_HandGrabbed = NULL;
	_bHandHoldingSwitch = false;
}
