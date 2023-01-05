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
#include "Items/ItemStorer.h"

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

	if (_bInStorer && _StoredIn)
	{
		SetActorTransform(_StoredIn->GetComponentTransform());
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

void AVRItem::BP_SetRootComponent(USceneComponent* NewRootComp)
{
	if (!NewRootComp)
		return;

	SetRootComponent(NewRootComp);
	//_MainGrabComponent->SetupAttachment(NewRootComp);
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

	//SetActorTickEnabled(false);

	SetOwner(Hand->GetOwner());

	_PHC->SetTargetObject(Hand->GetMotionControllerComponent());
	_PHC->SetPlayerControllerOwner(Cast<APlayerController>(GetOwner()));
	_PHC->EnableComponent(true);
	_PHC->SetTargetType(ETargetType::ETT_SetObject);

	_RootPrimitiveComponent->SetEnableGravity(false);

	if (_StoredIn)
	{
		if (!_bKeepWeaponStoredOnItemStorer)
		{
			_StoredIn->StoreItem(NULL);
			_StoredIn = NULL;
		}

		_RootPrimitiveComponent->SetSimulatePhysics(true);
		_bInStorer = false;
	}


	ComponentGrabbed(true, Hand->GetIsRightHand());

}

void AVRItem::MainGrabPointReleased(AVRHand* Hand)
{
	//SetActorTickEnabled(false);

	_PHC->EnableComponent(false);
	_PHC->SetTargetObject(NULL);
	_PHC->SetTargetType(ETargetType::ETT_NoTarget);
	

	if (!_RootPrimitiveComponent)
		return;

	if (_bKeepWeaponStoredOnItemStorer && _StoredIn)
	{
		_RootPrimitiveComponent->SetSimulatePhysics(false);
		_bInStorer = true;
		return;
	}
	else if (_ItemStorers.Num() > 0 && _ItemStorers[0])
	{
		if (_ItemStorers[0]->StoreItem(this))
		{
			_StoredIn = _ItemStorers[0];
			_RootPrimitiveComponent->SetSimulatePhysics(false);
			_bInStorer = true;
			return;
		}
	}

	_RootPrimitiveComponent->SetSimulatePhysics(true);
	_PHC->SetPlayerControllerOwner(NULL);
	SetOwner(NULL);
	_RootPrimitiveComponent->SetEnableGravity(true);


	ComponentGrabbed(false, Hand->GetIsRightHand());
}

void AVRItem::SetupItemRootComponent()
{
	UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(GetRootComponent());
	if (RootComp)
	{
		_PHC->SetPhysicsObject(RootComp);
		_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
		_PHC->SetTargetType(ETargetType::ETT_NoTarget);
		_RootPrimitiveComponent = RootComp;

		_RootPrimitiveComponent->OnComponentBeginOverlap.AddDynamic(this, &AVRItem::OnItemOverlapBegin);
		_RootPrimitiveComponent->OnComponentEndOverlap.AddDynamic(this, &AVRItem::OnItemOverlapEnd);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, GetName() + ": Can't Find RootComponent!");
	}
}

void AVRItem::OnItemOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UItemStorer* ST = Cast<UItemStorer>(OtherComp);

	if (!ST || !ST->ItemSizeAllowed(_ItemSize))
		return;

	_ItemStorers.Add(ST);

	OverlappedItemStorer(ST);
}

void AVRItem::OnItemOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UItemStorer* ST = Cast<UItemStorer>(OtherComp);
	int32 ID = _ItemStorers.Find(ST);

	if (ID > INDEX_NONE)
	{
		_ItemStorers.Remove(ST);
	}
}

void AVRItem::OverlappedItemStorer(UItemStorer* ItemStorer)
{
}
