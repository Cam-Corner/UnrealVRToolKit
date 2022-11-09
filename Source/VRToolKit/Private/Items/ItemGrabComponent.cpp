// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemGrabComponent.h"
#include "Online/NetworkHelpers.h"

UItemGrabComponent::UItemGrabComponent()
{
}

bool UItemGrabComponent::CanGrabComponent()
{
	if (_bGrabEnabled && !_HandHoldingComponent)
		return true;

	return false;
}

bool UItemGrabComponent::GrabComponent(AVRHand* HandGrabbing)
{
	if (!Super::GrabComponent(HandGrabbing))
		return false;

	_HandHoldingComponent = HandGrabbing;

	return true;
}

void UItemGrabComponent::ReleaseComponent(AVRHand* HandGrabbing)
{
	Super::ReleaseComponent(HandGrabbing);

	_HandHoldingComponent = NULL;
}


void UItemGrabComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemGrabComponent, _bGrabEnabled);
	DOREPLIFETIME(UItemGrabComponent, _HandHoldingComponent);
}