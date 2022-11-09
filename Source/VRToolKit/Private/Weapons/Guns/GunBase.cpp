// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Guns/GunBase.h"
#include "Items/ItemGrabComponent.h"
#include "Weapons/Guns/ReloadSystem.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Player/VRHand.h"
#include "Components/ActorComponent.h"
#include "Weapons/Guns/FirearmActions/FirearmAction.h"
#include "Weapons/Guns/ReloadSystem.h"
#include "Items/ItemStorer.h"

AGunBase::AGunBase()
{
}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();

	_MainGrabComponent->_ComponentTrigger.AddDynamic(this, &AGunBase::TriggerPressed);
	_MainGrabComponent->_ComponentBottomButton.AddDynamic(this, &AGunBase::BottomButtonPressed);
	_MainGrabComponent->_ComponentTopButton.AddDynamic(this, &AGunBase::TopButtonPressed);

	if (_bStartReloaded)
		HardReloadWeapon();

	/*TArray<UActorComponent*> _ActorComps;
	GetActorCom*/
}

void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleRecoil();
}

FGunTrigger* AGunBase::GetTriggerByIndex(int Index)
{
	/*if (Index >= _GunTriggers.Num())
		return nullptr;

	return &_GunTriggers[Index];*/

	return &_GunTrigger;
}

void AGunBase::SetReloadSystem(UReloadSystem* ReloadSystem)
{
	_ReloadSystem = ReloadSystem;
}

void AGunBase::AddRecoilOffset()
{
	_bInRecoilState = true;

	float FinalHorizontalOffset = _MaxHorizontalOffset;
	float FinalVerticalOffset = _MaxVerticalOffset;
	FVector FinalKickBackOffset = _MaxKickBackForce;
		
	if(_bRandomHorizontalRecoil)
		FinalHorizontalOffset = FMath::RandRange(_MinHorizontalOffset, _MaxHorizontalOffset);

	if (_bRandomVerticalRecoil)
		FinalVerticalOffset = FMath::RandRange(_MinVerticalOffset, _MaxVerticalOffset);

	if (_bRandomKickBack)
	{
		FinalKickBackOffset.X = FMath::RandRange(_MinKickBackForce.X, _MaxKickBackForce.X);
		FinalKickBackOffset.Y = FMath::RandRange(_MinKickBackForce.Y, _MaxKickBackForce.Y);
		FinalKickBackOffset.Z = FMath::RandRange(_MinKickBackForce.Z, _MaxKickBackForce.Z);
	}

	_GotoVerticalRecoil = _CurrentVerticalRecoil + FinalVerticalOffset;
	_bCanRecoverVerticalRecoil = false;
	//_CurrentVerticalRecoil = _GotoVerticalRecoil;

	_GotoHorizontalRecoil = FinalHorizontalOffset;
	_bCanRecoverHorizontalRecoil = false;

	_GotoKickBack = FinalKickBackOffset;
	_bCanRecoverKickBackRecoil = false;
	//_CurrentKickBack = _GotoKickBack;
}

void AGunBase::AddFirearmActionComponent(UFirearmAction* FirearmAction)
{
	_FirearmActions.Add(FirearmAction);
}

UClass* AGunBase::GetItemMagazine()
{
	if (_ReloadSystem)
		return _ReloadSystem->GetMagazineType();

	return nullptr;
}

void AGunBase::MainGrabPointGrabbed(AVRHand* Hand)
{
	Super::MainGrabPointGrabbed(Hand);

	for (UFirearmAction* FA : _FirearmActions)
	{
		FA->SetWeaponGrabbed(true);
	}
}

void AGunBase::MainGrabPointReleased(AVRHand* Hand)
{
	Super::MainGrabPointReleased(Hand);

	for (UFirearmAction* FA : _FirearmActions)
	{
		FA->SetWeaponGrabbed(false);
	}
}

void AGunBase::OverlappedItemStorer(UItemStorer* ItemStorer)
{
	Super::OverlappedItemStorer(ItemStorer);

	if (!_bCanHardReloadFromItemStorer)
		return;

	if (_ReloadSystem && _ReloadSystem->CanReload())
	{
		if (ItemStorer->GetItemStored() == this)
		{
			HardReloadWeapon();
		}
	}
}

void AGunBase::TriggerPressed(float Value)
{
	/*if (_GunTriggers.Num() <= 0)
		return;

	_GunTriggers[0]._PressedValue = Value;*/

	_GunTrigger._PressedValue = Value;

	if (Value > 0.9f && _GunTrigger._CanPress)
	{
		_GunTrigger._CanPress = false;
		_GunTrigger._TriggerPressed.Broadcast();
	}
	else if(Value < 0.1f && !_GunTrigger._CanPress)
	{
		_GunTrigger._CanPress = true;
		_GunTrigger._TriggerReleased.Broadcast();
	}
}

void AGunBase::BottomButtonPressed(bool bPressed)
{
	if (bPressed)
	{
		_BottomButtonPressed.Broadcast();
	}
}

void AGunBase::TopButtonPressed(bool bPressed)
{
	if (bPressed)
	{
		_TopButtonPressed.Broadcast();
	}
}

void AGunBase::EditWeaponOffset(float DeltaTime)
{
	//if (!_bInRecoilState)
		//Super::EditWeaponOffset(DeltaTime);
}

void AGunBase::HandleRecoilRecover()
{
	if (!GetWorld())
		return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	/*float FinalRecoilFollow = 1 - _RecoilFollowTime;
	_CurrentVerticalRecoil = FMath::Lerp(_CurrentVerticalRecoil, _GotoVerticalRecoil, 1 - FMath::Pow(FinalRecoilFollow, DeltaTime));
	_CurrentHorizontalRecoil = FMath::Lerp(_CurrentHorizontalRecoil, _GotoHorizontalRecoil, 1 - FMath::Pow(FinalRecoilFollow, DeltaTime));
	_CurrentKickBack = FMath::Lerp(_CurrentKickBack, _GotoKickBack, 1 - FMath::Pow(FinalRecoilFollow, DeltaTime));
	
	float FinalRecoilRecover = 1 - _RecoverTime;

	_GotoVerticalRecoil = FMath::Lerp(_GotoVerticalRecoil, 0, 1 - FMath::Pow(FinalRecoilRecover, DeltaTime));
	_GotoHorizontalRecoil = FMath::Lerp(_GotoHorizontalRecoil, 0, 1 - FMath::Pow(FinalRecoilRecover, DeltaTime));
	_GotoKickBack = FMath::Lerp(_GotoKickBack, FVector::ZeroVector, 1 - FMath::Pow(FinalRecoilRecover, DeltaTime));
	
	//Handle Vertical recoil
	ConsistantResetToValue(_GotoVerticalRecoil, 0, _RecoverTime);
	ConsistantResetToValue(_CurrentVerticalRecoil, _GotoVerticalRecoil, _RecoilFollowTime);

	//Handle horizontal recoil
	ConsistantResetToValue(_GotoHorizontalRecoil, 0, _RecoverTime);
	ConsistantResetToValue(_CurrentHorizontalRecoil, _GotoHorizontalRecoil, _RecoilFollowTime);

	//Handle Pushback recoil
	ConsistantResetToValue(_GotoKickBack, FVector::ZeroVector, _RecoverTime);
	ConsistantResetToValue(_CurrentKickBack, _GotoKickBack, _RecoilFollowTime);*/

	//Handle Horizontal recoil
	if (_bCanRecoverHorizontalRecoil)
		_CurrentHorizontalRecoil = FMath::FInterpTo(_CurrentHorizontalRecoil, 0, DeltaTime, _RecoverTime);
	else
	{
		_CurrentHorizontalRecoil = FMath::FInterpTo(_CurrentHorizontalRecoil, _GotoHorizontalRecoil, DeltaTime, _RecoilFollowTime);

		if (FMath::IsNearlyEqual(_CurrentHorizontalRecoil, _GotoHorizontalRecoil, 0.15f))
			_bCanRecoverHorizontalRecoil = true;
	}

	//Handle Vertical recoil
	if(_bCanRecoverVerticalRecoil)
		_CurrentVerticalRecoil = FMath::FInterpTo(_CurrentVerticalRecoil, 0, DeltaTime, _RecoverTime);
	else
	{
		_CurrentVerticalRecoil = FMath::FInterpTo(_CurrentVerticalRecoil, _GotoVerticalRecoil, DeltaTime, _RecoilFollowTime);
		
		if (FMath::IsNearlyEqual(_CurrentVerticalRecoil, _GotoVerticalRecoil, 0.15f))
			_bCanRecoverVerticalRecoil = true;
	}

	//Handle kickback recoil
	if (_bCanRecoverKickBackRecoil)
	{
		_CurrentKickBack.X = FMath::FInterpTo(_CurrentKickBack.X, 0, DeltaTime, _RecoverTime);
		_CurrentKickBack.Y = FMath::FInterpTo(_CurrentKickBack.Y, 0, DeltaTime, _RecoverTime);
		_CurrentKickBack.Z = FMath::FInterpTo(_CurrentKickBack.Z, 0, DeltaTime, _RecoverTime);

	}
	else
	{
		_CurrentKickBack.X = FMath::FInterpTo(_CurrentKickBack.X, _GotoKickBack.X, DeltaTime, _RecoilFollowTime);
		_CurrentKickBack.Y = FMath::FInterpTo(_CurrentKickBack.Y, _GotoKickBack.Y, DeltaTime, _RecoilFollowTime);
		_CurrentKickBack.Z = FMath::FInterpTo(_CurrentKickBack.Z, _GotoKickBack.Z, DeltaTime, _RecoilFollowTime);

		if (FVector::DistSquared(_CurrentKickBack, _GotoKickBack) < 0.15f)
			_bCanRecoverKickBackRecoil = true;
	}
}

void AGunBase::HandleRecoil()
{
	if (!_MainGrabComponent || !_MainGrabComponent->GetHand())
		return;

	FTransform HandT = _MainGrabComponent->GetHand()->GetTrackingHandTransform();
	FQuat HandRot = HandT.GetRotation();

	//FQuat VerticalOffset = FQuat(HandRot.GetRightVector(), FMath::DegreesToRadians(45/* + _CurrentVerticalRecoil*/));
	FQuat VerticalOffset = FQuat(FRotator(-45 + _CurrentVerticalRecoil, 0, 0));
	FQuat HorizontalOffset = FQuat(FRotator(0, _CurrentHorizontalRecoil, 0));

	/*FVector HorizontalAxis = FVector::CrossProduct(HandRot.GetForwardVector(), HandRot.GetRightVector()).GetSafeNormal();
	FQuat HorizontalOffset = FQuat(HorizontalAxis, FMath::DegreesToRadians(_CurrentHorizontalRecoil));

	/*_PHC->SetTargetRotationOffset(VerticalOffset * HorizontalOffset);

	FVector FinalLocOffset = GetActorForwardVector() * _CurrentKickBack.Y;
	FinalLocOffset += GetActorRightVector() * _CurrentKickBack.X;
	FinalLocOffset += GetActorUpVector() * _CurrentKickBack.Z;

	_PHC->SetTargetLocationOffset(FinalLocOffset);*/

	HandRot *= FQuat(FRotator(-45, 0, 0));
	FVector WristOffset = FVector::ZeroVector;
	WristOffset += HandRot.GetForwardVector() * _OffsetToWrist.X;
	WristOffset += HandRot.GetRightVector() * _OffsetToWrist.Y;
	WristOffset += HandRot.GetUpVector() * _OffsetToWrist.Z;

	FVector RotLoc = HandT.GetLocation() + WristOffset;
	FVector Distance = HandT.GetLocation() - RotLoc;
	//DrawDebugLine(GetWorld(), HandT.GetLocation(), RotLoc, FColor::Red, false, .1f);

	//New Horizontal Rotation offset
	FVector HorizontalRotation = Distance.RotateAngleAxis(-_CurrentHorizontalRecoil, HandRot.GetUpVector());
	FVector HorizontalMoveOffset = (RotLoc + HorizontalRotation) - GetActorLocation();

	//New Vertical Rotation offset
	FVector VerticalRotation = Distance.RotateAngleAxis(-_CurrentVerticalRecoil, HandRot.GetRightVector());
	FVector VerticalMoveOffset = (RotLoc + VerticalRotation) - GetActorLocation();

	//new Kickback offset
	FVector FinalKickbackOffset = HandRot.GetForwardVector() * _CurrentKickBack.X;
	FinalKickbackOffset += HandRot.GetRightVector() * _CurrentKickBack.Y;
	FinalKickbackOffset += HandRot.GetUpVector() * _CurrentKickBack.Z;

	_PHC->SetTargetLocationOffset(/*VerticalMoveOffset + HorizontalMoveOffset +*/ FinalKickbackOffset);
	_PHC->SetTargetRotationOffset(HorizontalOffset * VerticalOffset);

	HandleRecoilRecover();
}

void AGunBase::ConsistantResetToValue(float& CurrentValue, float GotoValue, float Alpha)
{
	if (!GetWorld())
		return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (CurrentValue > GotoValue)
	{
		CurrentValue -= Alpha * DeltaTime;

		if (CurrentValue < GotoValue)
			CurrentValue = GotoValue;
	}
	else if (CurrentValue < GotoValue)
	{
		CurrentValue += Alpha * DeltaTime;

		if (CurrentValue > GotoValue)
			CurrentValue = GotoValue;
	}
}

void AGunBase::ConsistantResetToValue(FVector& CurrentValue, FVector GotoValue, float Alpha)
{
	if (!GetWorld())
		return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (CurrentValue.X > GotoValue.X)
	{
		CurrentValue.X -= Alpha * DeltaTime;

		if (CurrentValue.X < GotoValue.X)
			CurrentValue.X = GotoValue.X;
	}
	else if (CurrentValue.X < GotoValue.X)
	{
		CurrentValue.X += Alpha * DeltaTime;

		if (CurrentValue.X > GotoValue.X)
			CurrentValue.X = GotoValue.X;
	}

	if (CurrentValue.Y > GotoValue.Y)
	{
		CurrentValue.Y -= Alpha * DeltaTime;

		if (CurrentValue.Y < GotoValue.Y)
			CurrentValue.Y = GotoValue.Y;
	}
	else if (CurrentValue.Y < GotoValue.Y)
	{
		CurrentValue.Y += Alpha * DeltaTime;

		if (CurrentValue.Y > GotoValue.Y)
			CurrentValue.Y = GotoValue.Y;
	}

	if (CurrentValue.Z > GotoValue.Z)
	{
		CurrentValue.Z -= Alpha * DeltaTime;

		if (CurrentValue.Z < GotoValue.Z)
			CurrentValue.Z = GotoValue.Z;
	}
	else if (CurrentValue.Z < GotoValue.Z)
	{
		CurrentValue.Z += Alpha * DeltaTime;

		if (CurrentValue.Z > GotoValue.Z)
			CurrentValue.Z = GotoValue.Z;
	}
}

void AGunBase::HardReloadWeapon()
{
	for (UFirearmAction* FA : _FirearmActions)
	{
		FA->HardReload();
	}
}
