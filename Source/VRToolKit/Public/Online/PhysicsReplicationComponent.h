// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsReplicationComponent.generated.h"

class USceneComponent;
class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct FPhysicsFrame
{
	GENERATED_BODY()

	/* The location at where the object was in the physics frame */
	FVector _Location = FVector::ZeroVector;

	/* the linear velocity at which the object was at this physics frame */
	FVector _Linear_Velocity = FVector::ZeroVector;

	/* the linear velocity at which the object was at this physics frame */
	FVector _Angular_Velocity = FVector::ZeroVector;

	/* the Angular velocity at which the object was at this physics frame */
	FQuat _Rotation;

	/* if we are matching the target, what is the goto location we are trying to reach */
	FVector _Goto_Location = FVector::ZeroVector;

	/* if we are matching a target, what is the goto rotation we are trying to reach */
	FQuat _Goto_Rotation;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTOOLKIT_API UPhysicsReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPhysicsReplicationComponent();

	/* set the physics object that is being replicated */
	UFUNCTION(BlueprintCallable)

	virtual void SetReplicatedPhysicsObject(UPrimitiveComponent* Comp) { _PhysicsObject = Comp; }

	/* Set the params for the component */
	void SetParams(bool bUseJitterBuffer, uint32 JitterBufferSize = 10) { bUseJitterBuffer = _bUseJitterBuffer; _JitterBufferSize = JitterBufferSize; }

	/* Update the component to send physics move */
	UFUNCTION(BlueprintCallable)
	void SendPhysicsTick(FPhysicsFrame PhysicsFrame);

	/* Get New Physics Frame */
	UFUNCTION(BlueprintCallable)
	FPhysicsFrame GetNewPhysicsFrame();

protected:
	UFUNCTION(Server, UnReliable)
		void Server_SendPhysicsUpdate(FVector Location, FQuat Rotation, FVector LVelocity, FVector AVelocity);

	UFUNCTION(NetMulticast, UnReliable)
		void NetMulticast_SendPhysicsUpdate(FVector Location, FQuat Rotation, FVector LVelocity, FVector AVelocity);

	/* Wether we should create a jitter buffer,
	* if true, we delay any updates till the jitter buffer fills up a bit(using the variable JitterBufferSize)
	* Can help make sure we always have physics input and make it look smoother but at the cost of a bit more delay
	*/
	UPROPERTY(EditAnywhere, Category = "Replication Settings")
		bool _bUseJitterBuffer = true;

	/* set how big the jitter buffer gets before we start executing the physics updates */
	UPROPERTY(EditAnywhere, Category = "Replication Settings")
		uint32 _JitterBufferSize = 5;

	/* The physics object that is being replicated */
	UPrimitiveComponent* _PhysicsObject = NULL;

	TArray<FPhysicsFrame> _JitterBuffer;

	bool _bWaitedForJitterBufferToFill = false;

};
