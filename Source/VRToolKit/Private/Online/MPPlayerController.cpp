// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/NetworkHelpers.h"
#include "Kismet/GameplayStatics.h"

AMPPlayerController::AMPPlayerController()
{

}

void AMPPlayerController::Server_SwapVRandNonVRPawn_Implementation(bool bUseVR)
{
	if (AMPGameMode* GM = Cast<AMPGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->SwitchVRandNonVR(this, bUseVR);
	}
}
