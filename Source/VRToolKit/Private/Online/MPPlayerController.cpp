// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/NetworkHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Online/NetworkHelpers.h"
#include "Online/MPPlayerController.h"

DEFINE_LOG_CATEGORY(LogAMPPlayerController);

AMPPlayerController::AMPPlayerController()
{

}

void AMPPlayerController::NF_Client_Disconnect_Implementation(const FString& Reason)
{
	ConsoleCommand("disconnect");
	UE_LOG(LogAMPPlayerController, Display, TEXT("Player Disconnected, Reason: %s "), *Reason);
}

void AMPPlayerController::Server_SwapVRandNonVRPawn_Implementation(bool bUseVR)
{
	if (AMPGameMode* GM = Cast<AMPGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->SwitchVRandNonVR(this, bUseVR);
	}
}
