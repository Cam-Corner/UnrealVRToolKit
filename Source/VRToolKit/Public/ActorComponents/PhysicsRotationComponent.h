// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utility/PIDControllers.h"
#include "PhysicsRotationComponent.generated.h"

class UPrimitiveComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTOOLKIT_API UPhysicsRotationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPhysicsRotationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetBaseRotationToLockTo(FQuat BaseRotation);

	UFUNCTION(BlueprintCallable)
	void SetMinMaxAngle(float MinAngle, float MaxAngle, bool bLockToAngle = true);

	UFUNCTION(BlueprintCallable)
	void SetLockToAngle(bool bLockToAngle);	

	UFUNCTION(BlueprintCallable)
	void SetLockedAxis(FVector LockedAxis, bool bLockRotationToAxis = true);
	
	UFUNCTION(BlueprintCallable)
	void SetLockRototationToAxis(bool bLockRotationToAxis);
	
	UFUNCTION(BlueprintCallable)
	void SetPhysicsComponentToMove(UPrimitiveComponent* PhysicsComp);
	
	UFUNCTION(BlueprintCallable)
	void SetGotoRotation(FQuat GotoRotation, bool bGotoRotation = true);
	
	UFUNCTION(BlueprintCallable)
	void SetCanGotoRotation(bool bGotoRotation);

private:
	UPROPERTY(EditAnywhere, Category = "Physics Rotation Settings")
	FQuat _BaseRotation = FQuat(FVector::UpVector, 0);
		
	UPROPERTY(EditAnywhere, Category = "Physics Rotation Settings")
	FQuat _GotoRotation = FQuat(FVector::UpVector, 0);

	UPROPERTY(EditAnywhere, Category = "Physics Rotation Settings")
	float _MinAngle = -5;
	
	UPROPERTY(EditAnywhere, Category = "Physics Rotation Settings")
	float _MaxAngle = 5;

	UPROPERTY(EditAnywhere, Category = "Physics Rotation Settings")
	bool _bLockToAngle = false;

	UPROPERTY(EditAnywhere, Category = "Physics Rotation Settings")
	bool _bLockRotationToAxis = false;

	UPROPERTY(EditAnywhere, Category = "Physics Rotation Settings")
	bool _bGotoRotation = false;

	UPROPERTY(EditAnywhere, Category = "Physics Rotation Settings")
	FVector _LockedAxis = FVector::ZeroVector;

	FVector _TargetLocationOffset = FVector::ZeroVector;

	FQuat _TargetRotationOffse = FQuat(FVector::ForwardVector, 0);

	UPrimitiveComponent* _PhysicsComp;

	FCalculateCustomPhysics _OnCalculateCustomPhysics;
	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);

	UPROPERTY(EditAnywhere, Category = "Physics Rotation Settings", meta = (AdvancedDisplay))
	FQuatPDController _QuatPID;

};
