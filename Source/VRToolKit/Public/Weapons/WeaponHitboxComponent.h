// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponHitboxComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTOOLKIT_API UWeaponHitboxComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponHitboxComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY()
	TArray<UPrimitiveComponent*> _HitBoxes;

	UPROPERTY(EditAnywhere, Category = "HitBoxes")
	TArray<FString> _HitBoxNames;
};
