// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/VRItem.h"
#include "..\..\Public\Items\VRItem.h"
#include "Items/ItemGrabComponent.h"
#include "Player/VRHand.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Online/NetworkHelpers.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "MotionControllerComponent.h"

// Sets default values
AVRItem::AVRItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_MainGrabComponent = CreateDefaultSubobject<UItemGrabComponent>("MainGrabComponent");
	//_MainGrabComponent->SetupAttachment(GetRootComponent());
	_MainGrabComponent->_ComponentGrabbed.AddDynamic(this, &AVRItem::MainGrabPointGrabbed);
	_MainGrabComponent->_ComponentReleased.AddDynamic(this, &AVRItem::MainGrabPointReleased);

	_PHC = CreateDefaultSubobject<UPhysicsHandlerComponent>("PHC");
}

// Called when the game starts or when spawned
void AVRItem::BeginPlay()
{
	Super::BeginPlay();

	SetupItemRootComponent();
}

// Called every frame
void AVRItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_MainGrabComponent->GetHand())
	{
		_PHC->SetDesiredVelocity(_MainGrabComponent->GetHand()->GetDesiredVelocity());
	}
}


void AVRItem::ForceDrop(bool bDestroyAfter)
{

	if (!_MainGrabComponent || !_MainGrabComponent->GetHand() || !_RootPrimitiveComponent)
		return;

	_MainGrabComponent->GetHand()->GripReleased();

	if (bDestroyAfter)
		Destroy();

	_RootPrimitiveComponent->SetEnableGravity(true);

}

bool AVRItem::IsBeingHeld()
{
	if (_MainGrabComponent && _MainGrabComponent->GetHand())
		return true;

	return false;
}

void AVRItem::MainGrabPointGrabbed(AVRHand* Hand)
{
	if (!Hand )
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, GetName() + ": Hand Input NULL!");
		return;
	}

	if (!_RootPrimitiveComponent)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, GetName() + ": _RootPrimitiveComponent NULL!");
		return;
	}

	SetOwner(Hand->GetOwner());

	_PHC->SetTargetObject(Hand->GetMotionControllerComponent());
	_PHC->SetPlayerControllerOwner(Cast<APlayerController>(GetOwner()));
	_PHC->EnableComponent(true);
	_PHC->SetTargetType(ETargetType::ETT_SetObject);

	_RootPrimitiveComponent->SetEnableGravity(false);
}

void AVRItem::MainGrabPointReleased(AVRHand* Hand)
{
	SetActorTickEnabled(false);

	_PHC->EnableComponent(false);
	_PHC->SetTargetObject(NULL);
	_PHC->SetPlayerControllerOwner(NULL);
	_PHC->SetTargetType(ETargetType::ETT_NoTarget);
	SetOwner(NULL);

	if (!_RootPrimitiveComponent)
		return;

	_RootPrimitiveComponent->SetEnableGravity(true);
}

void AVRItem::SetupItemRootComponent()
{
	UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(GetRootComponent());
	if (RootComp)
	{
		_PHC->SetReplicatedPhysicsObject(RootComp);
		_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
		_PHC->SetTargetType(ETargetType::ETT_NoTarget);
		_RootPrimitiveComponent = RootComp;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, GetName() + ": Can't Find RootComponent!");
	}
}
