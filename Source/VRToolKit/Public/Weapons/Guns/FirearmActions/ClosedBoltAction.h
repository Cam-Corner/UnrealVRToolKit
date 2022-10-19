// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Guns/FirearmActions/AutoLoadingAction.h"
#include "ClosedBoltAction.generated.h"

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API UClosedBoltAction : public UAutoLoadingAction
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UClosedBoltAction();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:



};
