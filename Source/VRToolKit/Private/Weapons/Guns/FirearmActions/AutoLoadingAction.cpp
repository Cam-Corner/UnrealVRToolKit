// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Guns/FirearmActions/AutoLoadingAction.h"
#include "Player/VRHand.h"
#include "Items/ItemGrabComponent.h"
#include "Weapons/Guns/GunBase.h"
#include "Weapons/Guns/GunAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Guns/ReloadSystem.h"

UAutoLoadingAction::UAutoLoadingAction()
{
	
}

void UAutoLoadingAction::BeginPlay()
{
	Super::BeginPlay();

	if (_FireModes.Num() > 0)
	{
		_CurrentFireMode = _FireModes[0];
	}
	else
	{
		FFireMode NoMode;
		NoMode._FireMode = EFireModes::EFM_NoneSelected;
		_CurrentFireMode = NoMode;
	}

	
}

void UAutoLoadingAction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (_bSliderGrabbed)
	{
		HandleSliderGrabbed(DeltaTime);	}
	else if (_bFiring)
	{
		HandleShooting(DeltaTime);
	}
	else if(_CurrentBoltMovePercentage > 0 && !_bHoldEmptyPos)
	{
		_CurrentBoltMovePercentage -= (100 * (_TimeBetweenShots * 2)) * DeltaTime;
		if (_CurrentBoltMovePercentage <= 0)
		{
			_CurrentBoltMovePercentage = 0;

			if (_bWasFiring)
			{
				bool PlaySoundAgain = false;
				switch (_CurrentFireMode._FireMode)
				{
				case EFireModes::EFM_NoneSelected:
					break;
				case EFireModes::EFM_Safety:
					break;
				case EFireModes::EFM_Automatic:
					if (_bRoundInChamber && _bTriggerPressed) 
					{ 
						_bFiring = true; 
						PlaySoundAgain = true; 
					}
					break;
				case EFireModes::EFM_Burst:
					if (_bRoundInChamber && _BurstShotsLeft > 0) 
					{ 
						_BurstShotsLeft--; 
						_bFiring = true;  
						PlaySoundAgain = true;
					}
					break;
				case EFireModes::EFM_SemiAuto:
					break;
				default:
					break;
				}

				if (_GunBase && PlaySoundAgain)
				{
					_GunBase->AddRecoilOffset();

					if (_FireSound)
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), _FireSound, _GunBase->GetActorLocation());
				}
			}
			
		}
	}

	HandleBoltPercentages();
}

void UAutoLoadingAction::SetHandGrabPos(USkeletalMeshComponent* SKMesh)
{
	Super::SetHandGrabPos(SKMesh);

	if (_HandGrabPose && _GunSKMesh)
	{
		FAttachmentTransformRules ATR = FAttachmentTransformRules::SnapToTargetNotIncludingScale;

		if (_bHasChargingHandle)
			_HandGrabPose->AttachToComponent(_GunSKMesh, ATR, _ChargingHandleBoneName);
		else
			_HandGrabPose->AttachToComponent(_GunSKMesh, ATR, _BoltBoneName);

		_HandGrabPose->SetVisibility(false);
	}
}

void UAutoLoadingAction::SliderGrabbed(AVRHand* Hand)
{
	if (!_GunBase || _bFiring)
		return;

	_bSliderGrabbed = true;
	_RelativeSliderGrabbedLocation = Hand->GetTrackingHandTransform().GetLocation() - _GunBase->GetActorLocation();
	_RelativeSliderGrabbedLocation += (_GunBase->GetActorRightVector() * (_BoltDistance * (_CurrentBoltMovePercentage / 100)));

	_HandGrabPose->SetVisibility(true);

}

void UAutoLoadingAction::SliderReleased(AVRHand* Hand)
{
	_bSliderGrabbed = false;

	_HandGrabPose->SetVisibility(false);
}

void UAutoLoadingAction::SetupComponentVariables()
{
	if (!_GunBase)
		return;

	//Bolt Variables
	_Sqrd_BoltLineLength = FVector::DistSquared(_GunBase->GetActorLocation(), 
		_GunBase->GetActorLocation() + (-_GunBase->GetActorForwardVector() * _BoltDistance));

	_Sqrd_BoltPerMoveOffset = _Sqrd_BoltLineLength / 100;

	//Charging Handle Variables
	_Sqrd_ChargingHandleLineLength = FVector::DistSquared(_GunBase->GetActorLocation(),
		_GunBase->GetActorLocation() + (-_GunBase->GetActorForwardVector() * _ChargingHandleDistance));

	_Sqrd_ChargingHandlePerMoveOffset = _Sqrd_ChargingHandleLineLength / 100;

	//Firing
	_TimeBetweenShots = _RateOfFire / 60;
}

void UAutoLoadingAction::HandleShooting(float DeltaTime)
{
	if (_bSliderGrabbed)
		return;

	_CurrentBoltMovePercentage += (100 * (_TimeBetweenShots * 2)) * DeltaTime;

	if (_CurrentBoltMovePercentage >= 100)
	{
		_bFiring = false;
		_CurrentBoltMovePercentage = 100;
	}
	
}

void UAutoLoadingAction::HandleSliderGrabbed(float DeltaTime)
{
	if (!_GunBase || !_SliderGrabComp || !_SliderGrabComp->GetHand())
		return;

	FVector LineStart = _GunBase->GetActorLocation() + _RelativeSliderGrabbedLocation;
	FVector LineEnd = LineStart;
	FVector Point = _SliderGrabComp->GetHand()->GetTrackingHandTransform().GetLocation();
	float GotoPerc = 0;

	if (_bHasChargingHandle)
	{
		LineEnd += -_GunBase->GetActorRightVector() * _ChargingHandleDistance;
		GotoPerc = GetPercentageOnLine(LineStart, LineEnd, Point, _Sqrd_ChargingHandleLineLength);
	}
	else
	{
		LineEnd += -_GunBase->GetActorRightVector() * _BoltDistance;
		GotoPerc = GetPercentageOnLine(LineStart, LineEnd, Point, _Sqrd_BoltLineLength);
	}

	//_CurrentBoltMovePercentage = FMath::Lerp(_CurrentBoltMovePercentage, GotoPerc, 1 - FMath::Pow(_GrabSlideLerpSpeed, DeltaTime));
	_CurrentBoltMovePercentage = GotoPerc;
	_bHoldEmptyPos = false;
}

void UAutoLoadingAction::SetSliderComp(UItemGrabComponent* Comp)
{
	_SliderGrabComp = Comp; 
	
	if (_SliderGrabComp)
	{
		_SliderGrabComp->_ComponentGrabbed.AddDynamic(this, &UAutoLoadingAction::SliderGrabbed);
		_SliderGrabComp->_ComponentReleased.AddDynamic(this, &UAutoLoadingAction::SliderReleased);

		if (_GunSKMesh)
		{
			if(_bHasChargingHandle)
				_SliderGrabComp->AttachToComponent(_GunSKMesh, FAttachmentTransformRules::KeepWorldTransform, _ChargingHandleBoneName);
			else
				_SliderGrabComp->AttachToComponent(_GunSKMesh, FAttachmentTransformRules::KeepWorldTransform, _BoltBoneName);
	
		}
	}
}

void UAutoLoadingAction::SetupComponent(AGunBase* GunBase, USkeletalMeshComponent* SKMesh)
{
	_GunBase = GunBase;
	_GunSKMesh = SKMesh;

	if (_GunBase)
	{
		FGunTrigger* Trigger = _GunBase->GetTriggerByIndex(0);
		if (Trigger)
		{
			Trigger->_TriggerPressed.AddDynamic(this, &UAutoLoadingAction::TriggerPressed);
			Trigger->_TriggerReleased.AddDynamic(this, &UAutoLoadingAction::TriggerReleased);

		}

		_GunBase->_BottomButtonPressed.AddDynamic(this, &UAutoLoadingAction::PrimaryButtonPressed);
		_GunBase->_TopButtonPressed.AddDynamic(this, &UAutoLoadingAction::ChangeFireMode);
	}

	SetupComponentVariables();
}

void UAutoLoadingAction::SetReloadSystem(UReloadSystem* ReloadSystem)
{
	_ReloadSystem = ReloadSystem;
}

void UAutoLoadingAction::TriggerPressed()
{
	if (_bFiring || _bSliderGrabbed || !_bRoundInChamber || _CurrentFireMode._FireMode == EFireModes::EFM_Safety)
		return;

	_bFiring = true;
	_bWasFiring = true;
	_bTriggerPressed = true;

	if (_GunBase)
	{
		_GunBase->AddRecoilOffset();

		if (_FireSound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), _FireSound, _GunBase->GetActorLocation());
	}

	_BurstShotsLeft = _CurrentFireMode._ShotPerBurst;
}

void UAutoLoadingAction::TriggerReleased()
{
	_bTriggerPressed = false;
}

void UAutoLoadingAction::PrimaryButtonPressed()
{
	if (!_ReloadSystem)
		return;

	if (_bHasQuickSliderRelease && _bCanQuickReleaseSlider && _ReloadSystem->HasBulletsLeft())
	{
		_bHoldEmptyPos = false;
		_bCanInsertRound = true;
		_bCanQuickReleaseSlider = false;
	}
	else if (_bQuickMagEject)
	{
		_ReloadSystem->ReleasedMag();
	}
}

void UAutoLoadingAction::ChangeFireMode()
{
	if (_FireModes.Num() <= 0)
		return;

	_CurrentFireModeSelected++;

	if (_CurrentFireModeSelected >= _FireModes.Num())
		_CurrentFireModeSelected = 0;

	_CurrentFireMode = _FireModes[_CurrentFireModeSelected];
}

void UAutoLoadingAction::HandleBoltPercentages()
{
	if (!_ReloadSystem)
		return;

	float Perc = _CurrentBoltMovePercentage;

	if (Perc >= 90)
	{
		if (_bCanEjectRound && _bRoundInChamber)
		{
			//ejectround visualaztion
			_bRoundInChamber = false;
			_bCanEjectRound = false;
		}

		if (_bCanPlaySliderBackSound)
		{
			_bCanPlaySliderBackSound = false;

			if (_SliderBackSound && _GunBase)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), _SliderBackSound, _GunBase->GetActorLocation());
		}
		_bCanInsertRound = true;
	}
	else if (Perc < 90)
	{
		if (!_bRoundInChamber && _bCanInsertRound)
		{
			if (_ReloadSystem->GetNextBullet())
			{
				_bRoundInChamber = true;
			}
			else if (_bWasFiring)
			{
				_bWasFiring = false;
				_CurrentBoltMovePercentage = _GunEmptyBoltRestingPerc;
				_CurrentChargingHandleMovePercentage = _GunEmptyChargingHandleRestingPerc;
				_bHoldEmptyPos = true;
				_bCanQuickReleaseSlider = true;
				if (_EmptySound && _GunBase)
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), _EmptySound, _GunBase->GetActorLocation());
			}

			_bCanPlaySliderBackSound = true;

			_bCanInsertRound = false;
		}	

		_bCanEjectRound = true;
	}
}
