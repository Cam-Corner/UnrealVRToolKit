// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MPGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAMPGameMode, Log, All);

class AMPPlayerController;
class APlayerController;

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API AMPGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AMPGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void HandlePlayerJoined(APlayerController* NewPlayer);

	UFUNCTION(BlueprintCallable)
	void SwitchVRandNonVR(APlayerController* Player, bool bUseVR);

private:
	TArray<AMPPlayerController*> _PlayerControllers;

	TSubclassOf<APawn> _NonVRPawn = nullptr;

	TSubclassOf<APawn> _VRPawn = nullptr;
};
