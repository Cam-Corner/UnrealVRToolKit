// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "HitBoxComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHitBoxHit, float, DamageDealt);

/**
 * 
 */
UCLASS(meta = (DisplayName = "HitBox", BlueprintSpawnableComponent))
class VRTOOLKIT_API UHitBoxComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:
	UHitBoxComponent();

	void HitBoxHit(float Damage);

	UPROPERTY(BlueprintAssignable)
	FHitBoxHit _HitBoxHit;
protected:
	UPROPERTY(EditAnywhere, Category = "HitBox Settings")
	float _DamageMultiplier = 1.f;

};
