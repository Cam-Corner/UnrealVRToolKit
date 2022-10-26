// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingSystem/EnvironmentGrabComponent.h"
#include "Player/VRHand.h"
#include "Utility/ExtraMaths.h"

// Sets default values for this component's properties
UEnvironmentGrabComponent::UEnvironmentGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	// ...
}

FVector UEnvironmentGrabComponent::GetLocationOnZone(FVector GotoLoc)
{
	float YBoxExtent = GetScaledBoxExtent().Y;
	FVector LineStart = GetComponentLocation() + (-GetRightVector() * YBoxExtent);
	FVector LineEnd = GetComponentLocation() + (GetRightVector() * YBoxExtent);

	return ExtraMaths::PointProjectionOnLine(LineStart, LineEnd, GotoLoc, true);
}


// Called when the game starts
void UEnvironmentGrabComponent::BeginPlay()
{
	Super::BeginPlay();
	_LastLocation = GetComponentLocation();
	// ...
}


// Called every frame
void UEnvironmentGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	_LastLocation = GetComponentLocation();
	// ...
}

FVector UEnvironmentGrabComponent::GetLocationDifferenceFromLastFrame()
{
	return GetComponentLocation() - _LastLocation;
}

void UEnvironmentGrabComponent::ForceHandRelease()
{
	for(AVRHand* Hand : _HandsHoldingComp)
	{
		//tell hand to release comp
	}

	_HandsHoldingComp.Empty();
}

bool UEnvironmentGrabComponent::GrabComponent(AVRHand* Hand)
{
	if (!Super::GrabComponent(Hand))
		return false;

	_HandsHoldingComp.Add(Hand);

	return true;
}

void UEnvironmentGrabComponent::ReleaseComponent(AVRHand* Hand)
{
	_HandsHoldingComp.Remove(Hand);
}