// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/NetworkHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "Player/VRCharacter.h"

DEFINE_LOG_CATEGORY(LogAMPGameMode);

AMPGameMode::AMPGameMode()
{
	{
		static ConstructorHelpers::FClassFinder<APawn> NonVRPlayerPawnOB(
			TEXT("/VRToolKit/Blueprints/Player/bp_VRCharacter"/*"/Game/Content/Blueprints/Player/BP_VRCharacter"*/));
		if (NonVRPlayerPawnOB.Succeeded())
		{
			DefaultPawnClass = NonVRPlayerPawnOB.Class;
			_NonVRPawn = NonVRPlayerPawnOB.Class;
		}
	}
	/*{
		static ConstructorHelpers::FClassFinder<APawn> VRPlayerPawnOB(
			TEXT("/Game/Content/Blueprints/Player/BP_NonVRCharacterInvert"));
		if (VRPlayerPawnOB.Succeeded())
		{
			_VRPawn = VRPlayerPawnOB.Class;
		}
	}*/
	
	PlayerControllerClass = AMPPlayerController::StaticClass();
	PlayerStateClass = AMPPlayerState::StaticClass();
	GameStateClass = AMPGameState::StaticClass();
}

void AMPGameMode::BeginPlay()
{
	Super::BeginPlay();

}

void AMPGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMPGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	if (_CurrentPlayerCount >= _MaxPlayerCount)
	{
		if (AMPPlayerController* MPController = Cast<AMPPlayerController>(NewPlayer))
		{
			MPController->NF_Client_Disconnect("Server Full!");
		}
	}

	HandlePlayerJoined(NewPlayer);
}

void AMPGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	
}

void AMPGameMode::Logout(AController* Exiting)
{
	if (AMPPlayerController* MPController = Cast<AMPPlayerController>(Exiting))
	{
		if (_PlayerControllers.Find(MPController))
		{
			_PlayerControllers.Remove(MPController);
			_MaxPlayerCount--;
		}
	}
}

void AMPGameMode::HandlePlayerJoined(AController* NewPlayer)
{
	if (AMPPlayerController* PC = Cast<AMPPlayerController>(NewPlayer))
	{
		_PlayerControllers.Add(PC);
		_CurrentPlayerCount++;
		UE_LOG(LogAMPGameMode, Warning, TEXT("Client Connected!"));
	}
}

void AMPGameMode::SwitchVRandNonVR(APlayerController* Player, bool bUseVR)
{
	if (!Player || !_VRPawn || !_NonVRPawn)
		return;

	/*APawn* PossedPawn = Player->GetPawn();
	Player->UnPossess();
	FVector SpawnLoc = PossedPawn ? PossedPawn->GetActorLocation() : FVector(0, 0, 0);
	FRotator SpawnRot = PossedPawn ? PossedPawn->GetActorRotation() : FRotator(0, 0, 0);
	PossedPawn->Destroy();

	if (bUseVR)
	{
		PossedPawn = GetWorld()->SpawnActor<APawn>(_VRPawn, SpawnLoc, SpawnRot);
	}
	else
	{
		PossedPawn = GetWorld()->SpawnActor<APawn>(_NonVRPawn, SpawnLoc, SpawnRot);
	}

	Player->Possess(PossedPawn);*/
}
