// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VRCharacterAB.h"

void UVRCharacterAB::SetLeftHandLocationAndRotation(FVector Location, FRotator Rotation)
{
	_LH_Location = Location;
	_LH_Rotation = Rotation;
}

void UVRCharacterAB::SetRightHandLocationAndRotation(FVector Location, FRotator Rotation)
{
	_RH_Location = Location;
	_RH_Rotation = Rotation;
}
