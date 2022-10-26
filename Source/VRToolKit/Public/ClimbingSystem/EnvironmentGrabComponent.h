// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/VRGrabComponent.h"
#include "EnvironmentGrabComponent.generated.h"

class AVRHand;

UCLASS( ClassGroup=(Custom), BlueprintType, meta=(BlueprintSpawnableComponent) )
class VRTOOLKIT_API UEnvironmentGrabComponent : public UVRGrabComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnvironmentGrabComponent();

	FVector GetLocationOnZone(FVector GotoLoc);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FVector GetLocationDifferenceFromLastFrame();

	void ForceHandRelease();

	virtual bool GrabComponent(AVRHand* Hand) override;

	virtual void ReleaseComponent(AVRHand* Hand) override;
private:
	FVector _LastLocation = FVector::ZeroVector;

	TArray<AVRHand*> _HandsHoldingComp;
};
