// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "VRGrabComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRGrabGrabbed, class AVRHand*, Hand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRGrabButtonPress, bool, bPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVRGrabAxis, float, Value);

class AVRHand;

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTOOLKIT_API UVRGrabComponent : public UBoxComponent
{
	GENERATED_BODY()
public:
	UVRGrabComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	virtual bool GrabComponent(AVRHand* HandGrabbing);

	virtual void ReleaseComponent(AVRHand* HandGrabbing);

	virtual bool CanGrabComponent();

	FVRGrabGrabbed _ComponentGrabbed;

	FVRGrabGrabbed _ComponentReleased;

	FVRGrabButtonPress _ComponentTopButton;

	FVRGrabButtonPress _ComponentBottomButton;

	FVRGrabAxis _ComponentTrigger;

	virtual void TriggerPressed(float Value);

	virtual void TopButtonPressed(bool bPressed);

	virtual void BottomButtonPressed(bool bPressed);
};
