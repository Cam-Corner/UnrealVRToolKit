#pragma once

#include "CoreMinimal.h"
#include "Utility/ExtraMaths.h"
#include "PIDControllers.generated.h"

/** Do Not Use This For Quaternions, Quaternions have their own unique PD Controller*/
template<typename T> struct FPDControllerT
{
public:
	FPDControllerT() {}

	T GetForce(float DeltaTime, T CurrentValue, T DesiredValue)
	{
		return InternalUpdate(DeltaTime, CurrentValue, DesiredValue);
	}

	void SetProportionalAndDerivative(float P, float D) { SetProportional(P); SetDerivative(D); }
	void SetProportional(float Value) { _P = Value; }
	void SetDerivative(float Value) { _D = Value; }
	void Reset() { _bDIntialized = false; }
protected:
	T InternalUpdate(float DeltaTime, T CurrentValue, T DesiredValue)
	{
		float PGain = _P;
		float DGain = _D;

		T ThisError = DesiredValue - CurrentValue;
		T ThisDerivative = (T)0;
		
		if (_bDIntialized)
			ThisDerivative = (ThisError - ErrorPrior) / DeltaTime;
		else
			_bDIntialized = true;

		T Output = (PGain * ErrorPrior) + (ThisDerivative * DGain);

		ErrorPrior = ThisError;

		return Output;

	}

	/**How fast it takes to reach the target */	
	float _P = 1.0f;

	/* 
	* Value = 1: Critically Damped
	* Value < 1: Under Damped and will oscillate depending on how low the value is
	* Value > 1: Over Damped and will be slow to reach the target 
	*/
	float _D = 1.0f;

	bool _bDIntialized = false;

private:
	T ErrorPrior;
};

USTRUCT(BlueprintType)
struct FPDController
{
	GENERATED_BODY()
public:
	FPDController() {}

	float GetForce(float DeltaTime, float CurrentValue, float DesiredValue)
	{
		PDController.SetProportionalAndDerivative(_P, _D);
		return PDController.GetForce(DeltaTime, CurrentValue, DesiredValue);
	}

public:
	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
	float _P = 1.0f;

	/** Changes how much it slows down towards the desired value */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
	float _D = 1.0f;

private:
	FPDControllerT<float> PDController;
};

USTRUCT(BlueprintType)
struct FPDController3D
{
	GENERATED_BODY()
public:
	FPDController3D() {}

	FVector GetForce(float DeltaTime, FVector CurrentValue, FVector DesiredValue)
	{
		_PDController.SetProportionalAndDerivative(_Frequency, _Dampening);
		return _PDController.GetForce(DeltaTime, CurrentValue, DesiredValue);
	}

	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _Frequency = 10.0f;

	/** Changes how much it slows down towards the desired value */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _Dampening = 1.0f;

	void Reset() { _PDController.Reset(); }
private:
	FPDControllerT<FVector> _PDController;
};

USTRUCT(BlueprintType)
struct FQuatPDController
{
	GENERATED_BODY()

public:
	FQuatPDController() {}

	/*
	* CQuat = Current Quaternion Value
	* DQuat = Desired Quaternion Value
	*/
	FVector GetTorque(float DeltaTime, FQuat CQuat, FQuat DQuat, FVector AVel, FVector InertiaTensor, FTransform ActorTransform)
	{
		float PGain = _PGain;// (6.f * _Frequency)* (6.f * _Frequency) * 0.25f;
		float DGain = _DGain;// 4.5f * _Frequency * _Dampening;

		FVector Axis = FVector::ZeroVector;
		float Angle = 0;
		FQuat Error = DQuat * CQuat.Inverse();

		if (Error.W < 0)
		{
			Error.X = -Error.X;
			Error.Y = -Error.Y;
			Error.Z = -Error.Z;
			Error.W = -Error.W;
		}

		Error.ToAxisAndAngle(Axis, Angle);
		Axis.Normalize();
		//Axis = FVector::DegreesToRadians(Axis);
		//Angle = FMath::DegreesToRadians(Angle);
		FVector P = PGain * Angle * Axis;
		FVector D = DGain * (AVel);
		FVector Value = P - D;

		{		
			//Value -= AVel;

			/*FVector VL = ActorTransform.InverseTransformPosition(Value);
			VL = InertiaTensor.ToOrientationQuat() * VL;
			VL = VL * InertiaTensor;
			VL = InertiaTensor.ToOrientationQuat().Inverse() * VL;
			FVector Force = ActorTransform.TransformPosition(VL);
			return Force * ForceMultiplier;*/

			/*FQuat RotInertia2World = FQuat::MakeFromEuler(InertiaTensor) * CQuat;
			Value = RotInertia2World.Inverse() * Value;
			Value *= InertiaTensor;
			Value = RotInertia2World * Value;*/
		}
		return Value * _Power;
	}

	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _PGain = .8f;

	/** Changes how much it slows down towards the desired value */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _DGain = .2f;

	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _Power = 1;
};

USTRUCT(BlueprintType)
struct FVelocityPDController
{
	GENERATED_BODY()
public:
	FVelocityPDController() {}

	FVector GetForce(float DeltaTime, FVector CurrentValue, FVector DesiredValue, FVector CurrentVelocity, FVector DesiredVelocity)
	{
		float PGain = (6.f * _Frequency) * (6.f * _Frequency) * 0.25f;
		float DGain = 4.5f * _Frequency * _Dampening;

		//float G = 1 / (1 + DGain * DeltaTime + PGain * DeltaTime * DeltaTime);
		/*float KSG = PGain * G;
		float KDG = (DGain + PGain * DeltaTime) * G;*/
		FVector F = (DesiredValue - CurrentValue) * PGain + (DesiredVelocity - CurrentVelocity) * DGain;
		//FVector F = (DesiredValue - CurrentValue) * KSG + (DesiredVelocity - CurrentVelocity) * KDG;
		return F;
	}

	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _Frequency = 6.f;

	/** Changes how much it slows down towards the desired value */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _Dampening = 1.0f;


};

USTRUCT(BlueprintType)
struct FPIDVectorController
{
	GENERATED_BODY()
public:
	FPIDVectorController() {}

	//Calculate the new add force from the PID Controller. Only need to use the velocity parameters if bUseVelocityForDTerm is true
	FVector GetForce(float DeltaTime, FVector CurrentValue, FVector DesiredValue, FVector CurrentVelocity = FVector::ZeroVector, FVector DesiredVelocity = FVector::ZeroVector)
	{
		//Calculate P Term
		FVector Error = DesiredValue - CurrentValue;
		FVector P = _PGain * Error;

		//calculate I Term
		_IStored = _IStored + (Error * DeltaTime);
		FVector I = _IGain * _IStored;

		//calculate D Term
		FVector D = FVector::ZeroVector;
		if (_bInitialzied)
		{
			if (_bUseVelocityForDTerm)
			{
				FVector VelocityRateOfChange = (CurrentVelocity - _LastVelocity) * DeltaTime;
				D = _DGain * (DesiredVelocity - CurrentVelocity);//-VelocityRateOfChange;
				_LastVelocity = CurrentVelocity;
			}
			else
			{
				FVector VelocityRateOfChange = (Error - _LastVelocity) / DeltaTime;
				D = _DGain * VelocityRateOfChange;
				_LastVelocity = Error;
			}
		}	
		_bInitialzied = true;

		//return value
		return (P + I + D) * _Power;
	}

	void Reset()
	{
		_bInitialzied = false;
	}

public:
	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _PGain = 1.0f;

	/**How fast it takes to reach the target */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _IGain = 1.0f;

	/** Changes how much it slows down towards the desired value */
	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _DGain = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _MaxIStored = 0;

	UPROPERTY(EditAnywhere, Category = "Tuning ")
		float _Power = 1;

	UPROPERTY(EditAnywhere, Category = "Tuning ")
		bool _bUseVelocityForDTerm = false;

private:
	FVector _LastVelocity = FVector::ZeroVector;

	bool _bInitialzied = false;

	FVector _IStored = FVector::ZeroVector;
};