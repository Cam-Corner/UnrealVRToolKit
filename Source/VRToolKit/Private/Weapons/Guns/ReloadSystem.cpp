// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Guns/ReloadSystem.h"
#include "Weapons/Guns/GunMagazine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UReloadSystem::UReloadSystem()
{
	OnComponentBeginOverlap.AddDynamic(this, &UReloadSystem::ReloadSystemOverlapEvent);
	PrimaryComponentTick.bCanEverTick = true;
	//PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UReloadSystem::GetNextBullet()
{
	if (_BulletsLeft <= 0 || !_bReadyToUse)
		return false;

	_BulletsLeft --;
	return true;
}

void UReloadSystem::ReleasedMag()
{
	if (!_bReadyToUse)
		return;

	_BulletsLeft = 0;

	_bReleasingMag = true;

	_bReadyToUse = false;

	SetComponentTickEnabled(true);

	if (_MagInsertSound && GetOwner())
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), _MagEjectSound, GetOwner()->GetActorLocation());
}

void UReloadSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{ 
	GEngine->AddOnScreenDebugMessage(389, .1f, FColor::Red, "ReloadSystemTicking!");
	if (_bReleasingMag)
	{
		_CurrentPercentage += (100 / _ReloadSpeed) * DeltaTime;
	
		if (_CurrentPercentage >= 100)
		{
			_CurrentPercentage = 100;
			
			if (!_bHideMagBone)
			{
				AGunMagazine* GunMag = GetWorld()->SpawnActor<AGunMagazine>(_AllowedMagazine, GetComponentLocation(), GetComponentRotation());
				if (GunMag)
				{
					GunMag->SetRemainingAmmo(_BulletsLeft);
				}
			}
			_bCanReload = true;
			_bHideMagBone = true;
			SetComponentTickEnabled(false);
		}
	}
	else
	{
		_CurrentPercentage -= (100 / _ReloadSpeed) * DeltaTime;
	
		if (_CurrentPercentage <= 0)
		{
			_CurrentPercentage = 0;
			_bReadyToUse = true;
			SetComponentTickEnabled(false);
		}
	}
}

bool UReloadSystem::HasBulletsLeft()
{
	if (_BulletsLeft > 0)
		return true;

	return false;
}

void UReloadSystem::ReloadSystemOverlapEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!_bCanReload)
		return;

	if (!OtherActor->IsA(_AllowedMagazine))
		return;

	if (AGunMagazine* Mag = Cast<AGunMagazine>(OtherActor))
	{
		if (Mag->IsBeingHeld())
		{
			_BulletsLeft = Mag->GetRemainingAmmo();
			_bCanReload = false;
			Mag->ForceDrop(true);
			_bReleasingMag = false;
			_bHideMagBone = false;
			SetComponentTickEnabled(true);

			if (_MagInsertSound && GetOwner())
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), _MagInsertSound, GetOwner()->GetActorLocation());
		}
	}
}
