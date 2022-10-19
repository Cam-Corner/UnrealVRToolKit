// Fill out your copyright notice in the Description page of Project Settings.


#include "Switches/LeverSwitch.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"

ALeverSwitch::ALeverSwitch()
{

}

void ALeverSwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	DrawDebugArc();
}

void ALeverSwitch::BeginPlay()
{
	Super::BeginPlay();

	UpdateCachedVariables();
}

void ALeverSwitch::DrawDebugArc()
{
	/*DrawDebugLine(GetWorld(), _CompLoc, _StartOfAngleLoc, FColor::Blue, false, .1f);
	DrawDebugLine(GetWorld(), _CompLoc, _EndOfAngleLoc, FColor::Blue, false, .1f);

	for (int i = 0; i < _ArcPoints.Num(); i++)
	{
		if (i > 0)
		{
			FVector Start = _ArcPoints[i - 1];
			FVector End = _ArcPoints[i];
			DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, .1f);

			if(!_bHandHoldingSwitch || (i != _ShortPointA || i != _ShortPointB))
				DrawDebugSphere(GetWorld(), _ArcPoints[i], 1, 32, FColor::Black, false, .1f);
		}
	}*/

	if (_bHandHoldingSwitch)
	{
		DrawDebugSphere(GetWorld(), _LastPointOnArc, 1.1f, 32, FColor::Green, false, .1f);
		
		if (_ShortPointA < _ArcPoints.Num() && _ShortPointB < _ArcPoints.Num())
		{
			DrawDebugSphere(GetWorld(), _ArcPoints[_ShortPointA], 1.1f, 32, FColor::Red, false, .1f);
			DrawDebugSphere(GetWorld(), _ArcPoints[_ShortPointB], 1.1f, 32, FColor::Red, false, .1f);
		}
	}
}

void ALeverSwitch::UpdateCachedVariables()
{
	_CompRot = _StaticSwitchBaseMesh->GetComponentQuat();
	_CompLoc = _MovingSwitchPartMesh->GetComponentLocation();

	if (!_bFlipAxisOfArc)
		_AxisOfArc = FVector::CrossProduct(_CompRot.GetUpVector(), _CompRot.GetForwardVector());
	else
		_AxisOfArc = FVector::CrossProduct(_CompRot.GetUpVector(), _CompRot.GetRightVector());


	_StartOfAngleLoc = _CompRot.GetUpVector();
	_StartOfAngleLoc = _StartOfAngleLoc.RotateAngleAxis(_MinAngle, _AxisOfArc);
	_StartOfAngleDir = _StartOfAngleLoc;
	_StartOfAngleLoc *= _RadiusOfArc;
	_StartOfAngleLoc += _CompLoc;

	_EndOfAngleLoc = _CompRot.GetUpVector();
	_EndOfAngleLoc = _EndOfAngleLoc.RotateAngleAxis(_MaxAngle, _AxisOfArc);
	_EndOfAngleDir = _EndOfAngleLoc;
	_EndOfAngleLoc *= _RadiusOfArc;
	_EndOfAngleLoc += _CompLoc;

	_ActualAngle = (_MaxAngle + 180) - (_MinAngle + 180);
	_ArcPoints.Empty();
	for (int i = 0; i < _SegmentsOfArc; i++)
	{
		FVector NewPoint = _StartOfAngleDir;
		NewPoint = NewPoint.RotateAngleAxis((_ActualAngle / (_SegmentsOfArc - 1)) * i, _AxisOfArc);
		NewPoint *= _RadiusOfArc + 0.5f;
		NewPoint += _CompLoc;
		_ArcPoints.Add(NewPoint);
	}
}

FVector ALeverSwitch::GetPointOnArc(FVector Location)
{
	float ClosestDistanceA = 999999999;
	float ClosestDistanceB = 999999998;
	_ShortPointA = 0;
	_ShortPointB = 0;

	for (int i = 0; i < _ArcPoints.Num(); i++)
	{
		float Dist = FVector::Dist(_ArcPoints[i], Location);

		if (Dist < ClosestDistanceA && Dist < ClosestDistanceB)
		{
			if (ClosestDistanceA > ClosestDistanceB)
			{
				ClosestDistanceA = Dist;
				_ShortPointA = i;
			}
			else
			{
				ClosestDistanceB = Dist;
				_ShortPointB = i;
			}
		}
		else if (Dist < ClosestDistanceA)
		{
			ClosestDistanceA = Dist;
			_ShortPointA = i;
		}
		else if (Dist < ClosestDistanceB)
		{
			ClosestDistanceB = Dist;
			_ShortPointB = i;
		}
	}

	if (_ShortPointA > _ArcPoints.Num() || _ShortPointB > _ArcPoints.Num())
		return FVector::ZeroVector;

	return ExtraMaths::PointProjectionOnLine(_ArcPoints[_ShortPointA], _ArcPoints[_ShortPointB],
		Location, true);
}

void ALeverSwitch::GotoAngle(float DeltaTime, float Angle)
{
	Angle = FMath::Clamp(Angle, _MinAngle, _MaxAngle);
	_CurrentAngle = Angle;
	FQuat Offset = FQuat(_AxisOfArc, FMath::DegreesToRadians(Angle));
	FQuat DesiredRot = Offset * _CompRot;
	FQuat NewQuat = DesiredRot;

	if(_bHandHoldingSwitch)
		NewQuat = FQuat::Slerp(_MovingSwitchPartMesh->GetComponentQuat(), DesiredRot, _HoldingSlerpSpeed * DeltaTime);
	
	_MovingSwitchPartMesh->SetWorldRotation(NewQuat);
}

void ALeverSwitch::HoldingSwitch(float DeltaTime, FVector HandLocation, FQuat HandRotation)
{
	FVector Loc = GetPointOnArc(HandLocation);
	_LastPointOnArc = Loc;

	FVector DirToHand = Loc - _CompLoc;
	DirToHand.Normalize();

	FVector AngleAxis = FVector::CrossProduct(_CompRot.GetUpVector(), _CompRot.GetForwardVector());
	float Angle = ExtraMaths::GetAngleOfTwoVectors(_StartOfAngleDir, DirToHand) + _MinAngle;
	GotoAngle(DeltaTime, Angle);

	_AngleVel = 0;
}

void ALeverSwitch::DefaultAction(float DeltaTime)
{
	float NewAngle = _CurrentAngle + (_AngleVel * DeltaTime);
	float Friction = _AngleFriction * _SimulatedWeight * DeltaTime;
	if (_AngleVel > 0)
	{	
		_AngleVel -= Friction;

		if (_AngleVel < 0)
			_AngleVel = 0;	
	}
	else
	{
		_AngleVel += Friction;

		if (_AngleVel > 0)
			_AngleVel = 0;
	}

	if (NewAngle < _MinAngle || NewAngle > _MaxAngle)
		_AngleVel = 0;

	GotoAngle(DeltaTime, NewAngle);
}

void ALeverSwitch::HitARigidBody(const FHitResult& Hit)
{
	float OtherVel = Hit.GetComponent()->GetPhysicsLinearVelocity().Size() / _SimulatedWeight;
	
	float HitAngle = ExtraMaths::GetAngleOfTwoVectors(_CompRot.GetForwardVector(), Hit.ImpactNormal);

	if (HitAngle > 90)
		_AngleVel -= OtherVel;
	else
		_AngleVel += OtherVel;
}
