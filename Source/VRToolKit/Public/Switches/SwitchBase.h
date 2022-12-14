// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/PIDControllers.h"
#include "SwitchBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSwitchEnabled, bool, Enabled);


class UStaticMeshComponent;
class UPhysicsRotationComponent;
class UItemGrabComponent;
class AVRHand;

UCLASS()
class VRTOOLKIT_API ASwitchBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASwitchBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable)
	FSwitchEnabled _SwitchEnabled;
protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* _StaticSwitchBaseMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* _MovingSwitchPartMesh;

	UPROPERTY(VisibleAnywhere)
	UItemGrabComponent* _GrabComp;

	virtual void HoldingSwitch(float DeltaTime, FVector HandLocation, FQuat HandRotation);
	virtual void DefaultAction(float DeltaTime);
	virtual void HitARigidBody(const FHitResult& Hit);

	bool _bHandHoldingSwitch = false;

	void SwitchEnabled(bool bEnabled);
private:
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void ComponentGrabbed(AVRHand* HandGrabbed);

	UFUNCTION()
		void ComponentReleased(AVRHand* Hand);

	AVRHand* _HandGrabbed;
};
