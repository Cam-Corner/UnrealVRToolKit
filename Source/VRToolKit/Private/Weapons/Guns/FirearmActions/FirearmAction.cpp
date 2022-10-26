// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Guns/FirearmActions/FirearmAction.h"
#include "Utility/ExtraMaths.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapons/Guns/GunBase.h"

// Sets default values for this component's properties
UFirearmAction::UFirearmAction()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UFirearmAction::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	if (AGunBase* GB = Cast<AGunBase>(GetOwner()))
	{
		GB->AddFirearmActionComponent(this);
	}
}


// Called every frame
void UFirearmAction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UFirearmAction::HardReload()
{

}

void UFirearmAction::SetWeaponGrabbed(bool bGrabbed)
{
	_bWeaponIsGrabbed = bGrabbed;
}

float UFirearmAction::GetPercentageOnLine(FVector StartPoint, FVector EndPoint, FVector Point, float UnsqauredLineLength)
{
	FVector NewPoint = ExtraMaths::PointProjectionOnLine(StartPoint, EndPoint, Point, true);

	if (UnsqauredLineLength < 0)
	{
		UnsqauredLineLength = FVector::DistSquared(StartPoint, EndPoint);
	}

	float DistToPoint = FVector::DistSquared(StartPoint, NewPoint);

	return (100 / UnsqauredLineLength) * DistToPoint;
}

