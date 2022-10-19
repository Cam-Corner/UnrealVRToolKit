// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "VRCharacterAB.generated.h"

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API UVRCharacterAB : public UAnimInstance
{
	GENERATED_BODY()

public:
	void SetLeftHandLocationAndRotation(FVector Location, FRotator Rotation);

	void SetRightHandLocationAndRotation(FVector Location, FRotator Rotation);

protected:
	UPROPERTY(BlueprintReadOnly)
		FVector _LH_Location;

	UPROPERTY(BlueprintReadOnly)
		FRotator _LH_Rotation;

	UPROPERTY(BlueprintReadOnly)
		FVector _RH_Location;

	UPROPERTY(BlueprintReadOnly)
		FRotator _RH_Rotation;
};
