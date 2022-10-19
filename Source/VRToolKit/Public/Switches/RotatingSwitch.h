// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Switches/SwitchBase.h"
#include "RotatingSwitch.generated.h"

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API ARotatingSwitch : public ASwitchBase
{
	GENERATED_BODY()

public:
	ARotatingSwitch();

protected:
	bool _bLockToAxis = false;
	FVector _AxisOfRotation = FVector::ZeroVector;
};
