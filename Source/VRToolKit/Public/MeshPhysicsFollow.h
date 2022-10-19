// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/PIDControllers.h"
#include "MeshPhysicsFollow.generated.h"

struct NewPIDRot
{
public:
	float _KP = 0;
	float _KI = 0;
	float _KD = 0;

	float GetTorque(float CurrentAngle, float TargetAngle, float DT)
	{

		//Error = Target - Current
		//P = Error * _KP
		//I = IStored + (Error * dt); //(should be clamped)  
		//D = (Error - ErrorLast) / dt
		//Result = P + I + D; (should be clamped)


		//RotationPID
		float Error = AngleDifference(TargetAngle, CurrentAngle);
		GEngine->AddOnScreenDebugMessage(3, 60.f, FColor::Yellow, "Error: " + FString::SanitizeFloat(Error));
		//_ErrorLast = Error;

		float P = _KP * Error;

		//_IntegralStored = FMath::Clamp(_IntegralStored + (Error * DT), -1, 1);
		//float I = _KI * _IntegralStored;

		float D = _KD * AngleDifference(Error, _ErrorLast) / DT;
		_ErrorLast = Error;

		return P /* + I*/ + D;
	}

	float AngleDifference(float A, float B)
	{
		float Result = (A - B) + 540;
		Result = (int)Result % 360;
		Result -= 180;
		return  Result;
	}

private:
	float _ErrorLast = 0;
	float _IntegralStored = 0;
};

struct NewTorsionSpring
{
public:
	float _Q = 0;
	float _P = 0;

	float GetTorque(float CurrentAngle, float DesiredAngle, float CurrentAVel, float DesiredAVel, FVector AxisOfRotation, FVector AxisOfAVel, float DT)
	{
		float AngleError = DesiredAngle - CurrentAngle;
		float AVelError = DesiredAVel - CurrentAVel;

		FVector T = _Q * AngleError * AxisOfRotation + _P * AVelError * AxisOfAVel;
	}

};

UCLASS()
class VRTOOLKIT_API AMeshPhysicsFollow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeshPhysicsFollow();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* _FollowMesh;
	
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* _PhysicsMesh;

	UPROPERTY(EditAnywhere);
	class UPhysicsHandlerComponent* _PHC;


	UPROPERTY(EditAnywhere, Category = "PDController")
	FQuatPDController _QuatPD;

	UPROPERTY(EditAnywhere, Category = "Torque PIDController")
		bool _bUseSubStepping = false;

	UPROPERTY(EditAnywhere, Category = "Torque PIDController")
		float _KP = 0;

	UPROPERTY(EditAnywhere, Category = "Torque PIDController")
		float _KI = 0;

	UPROPERTY(EditAnywhere, Category = "Torque PIDController")
		float _KD = 0;

	float AngleDifference(float A, float B);

	float _ErrorLast = 0;
	FVector _FErrorLast = FVector::ZeroVector;

	NewPIDRot _XPID;
	NewPIDRot _YPID;
	NewPIDRot _ZPID;

	FCalculateCustomPhysics _OnCalculateCustomPhysics;

	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);

	float _CheckTimer = 1;
	int _StepsRan = 0;

	FVector NewQuatPID(FQuat From, FQuat To, FVector AVel);
};