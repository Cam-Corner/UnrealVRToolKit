// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "AmmoStorageComponent.generated.h"

/**
 * 
 */
class AVRHand;
class AVRItem;

UCLASS(meta = (DisplayName = "AmmoStorage", BlueprintSpawnableComponent))
class VRTOOLKIT_API UAmmoStorageComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UAmmoStorageComponent();

	void SetHands(AVRHand* LeftHand, AVRHand* RightHand);

	AVRItem* GetMagazine(AVRHand* HandWantingMag);
protected:
	UPROPERTY(Editanywhere, Category = "Ammo Storage Settings")
	TArray<TSubclassOf<AVRItem>> _AllowedItemSpawns;

	UPROPERTY()
	AVRHand* _LeftHand;

	UPROPERTY()
	AVRHand* _RightHand;
};
