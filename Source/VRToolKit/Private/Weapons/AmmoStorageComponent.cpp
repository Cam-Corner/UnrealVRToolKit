// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/AmmoStorageComponent.h"
#include "Player/VRHand.h"
#include "Items/VRItem.h"

UAmmoStorageComponent::UAmmoStorageComponent()
{

}

void UAmmoStorageComponent::SetHands(AVRHand* LeftHand, AVRHand* RightHand)
{
	_LeftHand = LeftHand;
	_RightHand = RightHand;
}

AVRItem* UAmmoStorageComponent::GetMagazine(AVRHand* HandWantingMag)
{
	if (!_LeftHand || !_RightHand || !HandWantingMag)
	{

		GEngine->AddOnScreenDebugMessage(755, 10.f, FColor::Green, "AmmoGrab: Hands NULL");
		return NULL;
	}

	UClass* MagSpawn = NULL;
	FVector SpawnLocation = FVector::ZeroVector;
	FQuat SpawnRotation = FQuat(1);

	if (HandWantingMag == _LeftHand)
	{
		if (!_RightHand->GetHoldingItem())
		{
			GEngine->AddOnScreenDebugMessage(755, 10.f, FColor::Green, "AmmoGrab: GetHoldingItem NULL Right Hand");
			return NULL;	
		}

		MagSpawn = _RightHand->GetHoldingItem()->GetItemMagazine();
		
		SpawnLocation = _RightHand->GetActorLocation();
		SpawnRotation = _RightHand->GetActorQuat();
	}
	else if (HandWantingMag == _RightHand)
	{
		if (!_LeftHand->GetHoldingItem())
		{
			GEngine->AddOnScreenDebugMessage(755, 10.f, FColor::Green, "AmmoGrab: GetHoldingItem NULL Left Hand");
			return NULL;
		}

		MagSpawn = _LeftHand->GetHoldingItem()->GetItemMagazine();

		SpawnLocation = _LeftHand->GetActorLocation();
		SpawnRotation = _LeftHand->GetActorQuat();
	}

	if (!MagSpawn)
	{
		GEngine->AddOnScreenDebugMessage(755, 10.f, FColor::Green, "AmmoGrab: Mag was null");
		return NULL;
	}

	bool CanSpawn = false;
	for (TSubclassOf<AVRItem> Item : _AllowedItemSpawns)
	{
		if (MagSpawn == Item->GetClass())
			CanSpawn = true;
	}

	if (CanSpawn)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (_AllowedItemSpawns.Num() > 0)
		{
			AVRItem* SpawnedItem = GetWorld()->SpawnActor<AVRItem>(_AllowedItemSpawns[0], SpawnLocation, SpawnRotation.Rotator(), SpawnParams);

			if (!SpawnedItem)
			{
				GEngine->AddOnScreenDebugMessage(755, 10.f, FColor::Green, "AmmoGrab: Couldnt spawn item magazine");
			}

			return SpawnedItem;
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(755, 10.f, FColor::Green, "AmmoGrab: can spawn was false");
	}

	return NULL;
}
