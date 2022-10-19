// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/VRGrabComponent.h"
#include "ItemGrabComponent.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class VRTOOLKIT_API UItemGrabComponent : public UVRGrabComponent
{
	GENERATED_BODY()
public:
	UItemGrabComponent();

	void SetGrabEnabled(bool bGrabbable) { _bGrabEnabled = bGrabbable; }

	virtual bool CanGrabComponent() override;

	virtual bool GrabComponent(AVRHand* HandGrabbing) override;

	virtual void ReleaseComponent(AVRHand* HandGrabbing) override;

	AVRHand* GetHand() { return _HandHoldingComponent; }
private:
	UPROPERTY(Replicated)
		bool _bGrabEnabled = true;

	UPROPERTY(Replicated)
		AVRHand* _HandHoldingComponent;
};
