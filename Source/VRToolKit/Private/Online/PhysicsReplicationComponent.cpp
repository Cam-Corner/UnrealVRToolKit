// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/PhysicsReplicationComponent.h"

// Sets default values for this component's properties
UPhysicsReplicationComponent::UPhysicsReplicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
	// ...
}


void UPhysicsReplicationComponent::Server_SendPhysicsUpdate_Implementation(FVector Location, FQuat Rotation, FVector LVelocity, FVector AVelocity)
{
	NetMulticast_SendPhysicsUpdate(Location, Rotation, LVelocity, AVelocity);
}

void UPhysicsReplicationComponent::SendPhysicsTick(FPhysicsFrame PhysicsFrame)
{
	if (!_PhysicsObject)
		return;
	
	FPhysicsFrame PF = PhysicsFrame;

	Server_SendPhysicsUpdate(PF._Location, PF._Rotation, PF._Linear_Velocity, PF._Angular_Velocity);

}

FPhysicsFrame UPhysicsReplicationComponent::GetNewPhysicsFrame()
{
	/*
	if (!_bWaitedForJitterBufferToFill)
	{
		if (_JitterBuffer.Num() >= (int32)_JitterBufferSize)
		{
			_bWaitedForJitterBufferToFill = true;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(29, 0.1f, FColor::Red, "Waiting for buffer to fill (GetNewPhysicsFrame)!");
			return FPhysicsFrame();
		}
	}*/

	if (_JitterBuffer.Num() < (int32)_JitterBufferSize)
	{
		return FPhysicsFrame();	
	}

	FPhysicsFrame PF = _JitterBuffer[0];
	_JitterBuffer.RemoveAt(0);
	return PF;
}

void UPhysicsReplicationComponent::NetMulticast_SendPhysicsUpdate_Implementation(FVector Location, FQuat Rotation, FVector LVelocity, FVector AVelocity)
{
	FPhysicsFrame PF;
	PF._Location = Location;
	PF._Rotation = Rotation;
	PF._Linear_Velocity = LVelocity;
	PF._Angular_Velocity = AVelocity;
	_JitterBuffer.Add(PF);
}


