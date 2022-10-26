// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Switches/SwitchBase.h"
#include "LeverSwitch.generated.h"

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API ALeverSwitch : public ASwitchBase
{
	GENERATED_BODY()
public:
		ALeverSwitch();
		// Called every frame
		virtual void Tick(float DeltaTime) override;

		float GetCurrentAngle();

		float GetScaledAngle();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "SwitchSettings", meta = (ClampMin = -90, ClampMax = 90, UIMin = -90, UIMax = 90))
		float _MinAngle = -45;

	UPROPERTY(EditAnywhere, Category = "SwitchSettings", meta = (ClampMin = -90, ClampMax = 90, UIMin = -90, UIMax = 90))
		float _MaxAngle = 45;

	UPROPERTY(EditAnywhere, Category = "SwitchSettings")
		float _RadiusOfArc = 15;

	UPROPERTY(EditAnywhere, Category = "SwitchSettings")
		float _SegmentsOfArc = 15;

	UPROPERTY(EditAnywhere, Category = "SwitchSettings", meta = (ClampMin = -90, ClampMax = 90, UIMin = -90, UIMax = 90))
		float _RestingAngle = 0;

	UPROPERTY(EditAnywhere, Category = "SwitchSettings", DisplayName = "Flip Axis Of Arc")
		bool _bFlipAxisOfArc = false;

	UPROPERTY(EditAnywhere, Category = "SwitchSettings")
		float _HoldingSlerpSpeed = 3.f;

	UPROPERTY(EditAnywhere, Category = "SwitchSettings")
		float _ArchRotationOffsetFromUpVector = 90.f;

	/* Wethere we should update the cached variables every frame, wasted performance on objects that dont move */
	UPROPERTY(EditAnywhere, Category = "SwitchSettings", DisplayName = "Should Update Cached Variables")
	bool _bShouldUpdateCachedVariables = false;

	//If false, the switch will be enalbed when its at/near its max angle and then disabled when it isn't,
	//if true, it will enabled when its at/near its max angle, and then stay enabled even it it stays out this zone, then when its gets near the 
	//zone again, it will disable.
	UPROPERTY(EditAnywhere, Category = "SwitchSettings")
		bool _bIsToggleSwitch = false;

//Simulated Angle Physics
	UPROPERTY(EditAnywhere, Category = "SwitchSettings")
		float _SimulatedWeight = 5.f;

	UPROPERTY(EditAnywhere, Category = "SwitchSettings")
		float _AngleFriction = 2.f;

//Non UProperty varaibles
	FQuat _CompRot = FQuat(FVector::UpVector, 0);
	FVector _CompLoc = FVector::ZeroVector;
	FVector _AxisOfArc = FVector::ZeroVector;

	FVector _StartOfAngleLoc = FVector::ZeroVector;
	FVector _StartOfAngleDir = FVector::ZeroVector;

	FVector _EndOfAngleLoc = FVector::ZeroVector;
	FVector _EndOfAngleDir = FVector::ZeroVector;
	FVector _LastPointOnArc = FVector::ZeroVector;

	TArray<FVector> _ArcPoints;
	float _ActualAngle = 0;

	float _CurrentAngle = 0;

	float _AngleVel = 0;

	float _DrawUpdate = 0;

	int _ShortPointA = 0;
	int _ShortPointB = 0;

	bool _bCurrentlyEnabled = false;

	bool _bWhereOverAngle = false;
protected:
	void DrawDebugArc();

	void UpdateCachedVariables();

	FVector GetPointOnArc(FVector Location);

	void GotoAngle(float DeltaTime, float Angle);

	virtual void HoldingSwitch(float DeltaTime, FVector HandLocation, FQuat HandRotation) override;

	virtual void DefaultAction(float DeltaTime) override;

	virtual void HitARigidBody(const FHitResult& Hit) override;
};
