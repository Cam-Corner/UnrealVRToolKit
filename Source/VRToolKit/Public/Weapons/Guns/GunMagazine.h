// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/VRItem_StaticMesh.h"
#include "GunMagazine.generated.h"


UCLASS()
class VRTOOLKIT_API AGunMagazine : public AVRItem_StaticMesh
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGunMagazine();

public:	
	uint8 GetRemainingAmmo() { return _StartingAmmoAmount; }
	void SetRemainingAmmo(uint8 RemainingAmmo) { _StartingAmmoAmount = RemainingAmmo; }


protected:
	UPROPERTY(EditAnywhere)
	uint8 _StartingAmmoAmount = 7;

};
