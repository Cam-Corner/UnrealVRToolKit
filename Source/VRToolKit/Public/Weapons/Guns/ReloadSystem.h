// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "ReloadSystem.generated.h"

UENUM(BlueprintType)
enum EReloadType
{
	ERT_ExternalMagazine	UMETA(DisplayName = "ExternalMagazine"),
	ERT_InternalMagazine	UMETA(DisplayName = "InternalMagazine"),
	ERT_Chambering			UMETA(DisplayName = "Chambering"),
};

class AGunMagazine;
class USoundCue;

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class VRTOOLKIT_API UReloadSystem : public UBoxComponent
{
	GENERATED_BODY()
public:
	UReloadSystem();

	bool GetNextBullet();

	UFUNCTION()
	void ReleasedMag();

	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	UFUNCTION(BlueprintCallable)
	float GetReloadPercentage() { return  _CurrentPercentage; }

	UFUNCTION(BlueprintCallable)
	bool ShouldHide() { return _bHideMagBone; }

	bool HasBulletsLeft();

private:
	uint8 _BulletsLeft = 0;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagazineSystem")
		FName _BoneMagazineName = "b_gun_magazine";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagazineSystem")
	TSubclassOf<AGunMagazine> _AllowedMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagazineSystem")
		uint8 _MaxAmmoForInternalMagazine = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagazineSystem")
		float _ReloadSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagazineSystem: Audio")
		USoundCue* _MagInsertSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagazineSystem: Audio")
		USoundCue* _MagEjectSound;

	

	UFUNCTION()
	void ReloadSystemOverlapEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	bool _bCanReload = true;

	bool _bReadyToUse = false;

	bool _bReleasingMag = true;

	bool _bHideMagBone = true;

	float _CurrentPercentage = 100;
};
