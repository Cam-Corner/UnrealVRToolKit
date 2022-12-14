// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAMPPlayerController, Log, All);

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMPPlayerController();

	UFUNCTION(BlueprintCallable, Server, Reliable)
		void Server_SwapVRandNonVRPawn(bool bUseVR);

	UFUNCTION(BlueprintCallable, Client, Reliable)
		void NF_Client_Disconnect(const FString& Reason);
};
