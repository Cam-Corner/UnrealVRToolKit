// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Guns/FirearmActions/FirearmAction.h"
#include "AutoLoadingAction.generated.h"

class UItemGrabComponent;
class AVRHand; 
class UGunAnimInstance;
class AGunBase;
class USkeletalMeshComponent;
class USoundCue;
class UReloadSystem;

UENUM()
enum EFireModes
{
	EFM_NoneSelected	UMETA(DisplayName = "EFM_NoneSelected"),
	EFM_Safety			UMETA(DisplayName = "Safety"),
	EFM_SemiAuto		UMETA(DisplayName = "SemiAutomatic"),
	EFM_Automatic		UMETA(DisplayName = "Automatic"),
	EFM_Burst			UMETA(DisplayName = "Burst"),
};

USTRUCT(BlueprintType)
struct FFireMode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Firemode Info")
	TEnumAsByte<EFireModes> _FireMode = EFireModes::EFM_Safety;

	UPROPERTY(EditAnywhere, Category = "Firemode Info")
	float _AngleOfFireSelector = 0;

	UPROPERTY(EditAnywhere, Category = "Firemode Info")
	uint8 _ShotPerBurst = 1;
};

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class VRTOOLKIT_API UAutoLoadingAction : public UFirearmAction
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAutoLoadingAction();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SetHandGrabPos(USkeletalMeshComponent* SKMesh) override;

	virtual void HardReload() override;

	virtual void SetWeaponGrabbed(bool bGrabbed) override;
protected:
	UFUNCTION()
	void SliderGrabbed(AVRHand* Hand);

	UFUNCTION()
	void SliderReleased(AVRHand* Hand);

	void SetupComponentVariables();

	void HandleShooting(float DeltaTime);

	void HandleSliderGrabbed(float DeltaTime);

	UFUNCTION(BlueprintCallable)
		void SetSliderComp(UItemGrabComponent* Comp);

	UFUNCTION(BlueprintCallable)
		void SetupComponent(AGunBase* GunBase, USkeletalMeshComponent* SKMesh);

	UFUNCTION(BlueprintCallable)
		void SetReloadSystem(UReloadSystem* ReloadSystem);

	UFUNCTION()
		void TriggerPressed();

	UFUNCTION()
		void TriggerReleased();

	UFUNCTION()
	void PrimaryButtonPressed();

	UFUNCTION()
		void ChangeFireMode();

	/* Blueprint getter functions used for passing variables to the animation BP */
protected:
	UFUNCTION(BlueprintCallable)
		float GetSliderPercentage() { return _CurrentBoltMovePercentage; }

	UFUNCTION(BlueprintCallable)
		float GetCharginHandlePercentage() { return _CurrentChargingHandleMovePercentage; }

	void HandleBoltPercentages();
protected:
	//Auto loading Settings
	UPROPERTY(EditAnywhere, Category = "AutoLoadingActionSettings")
	float _RateOfFire = 120;

	UPROPERTY(EditAnywhere, Category = "AutoLoadingActionSettings")
	TArray<FFireMode> _FireModes;

	UPROPERTY(EditAnywhere, Category = "AutoLoadingActionSettings", meta = (DisplayName = "QuickMagEject"))
	bool _bQuickMagEject = true;

	UPROPERTY(EditAnywhere, Category = "AutoLoadingActionSettings", meta = (DisplayName = "QuickSliderRelease"))
		bool _bHasQuickSliderRelease = true;

	UPROPERTY(EditAnywhere, Category = "AutoLoadingActionSettings")
		float _GrabSlideLerpSpeed = .25f;

	//Bolt Settings
	UPROPERTY(EditAnywhere, Category = "BoltSettings")
	FName _BoltBoneName = "b_gun_bolt";

	UPROPERTY(EditAnywhere, Category = "BoltSettings")
	float _BoltDistance = 5;

	UPROPERTY(EditAnywhere, Category = "BoltSettings", meta = (MinValue = 0, MaxValue = 100))
	float _GunEmptyBoltRestingPerc = 0;

	//Charging Handle Settings
	UPROPERTY(EditAnywhere, Category = "ChargingHandleSettings", DisplayName = "HasChargingHandle")
	bool _bHasChargingHandle = false;

	UPROPERTY(EditAnywhere, Category = "ChargingHandleSettings", meta = (EditCondition = "_bHasChargingHandle"))
	FName _ChargingHandleBoneName = "b_gun_charginghandle";

	UPROPERTY(EditAnywhere, Category = "ChargingHandleSettings", meta = (EditCondition = "_bHasChargingHandle"))
	float _ChargingHandleDistance = 5.f;

	UPROPERTY(EditAnywhere, Category = "ChargingHandleSettings", meta = (MinValue = 0, MaxValue = 100))
	float _GunEmptyChargingHandleRestingPerc = 0;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundCue* _FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundCue* _SliderBackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundCue* _EmptySound;

	UPROPERTY(EditAnywhere, Category = "Firemode Info")
		TEnumAsByte<EFireModes> _FireMode;

protected:
	//Slider Variables
	UItemGrabComponent* _SliderGrabComp;
	bool _bSliderGrabbed = false;
	FVector _RelativeSliderGrabbedLocation = FVector::ZeroVector;

	//Bolt Variables
	float _Sqrd_BoltPerMoveOffset = 0;	
	float _Sqrd_BoltLineLength = 0;
	float _CurrentBoltMovePercentage = 0;

	//Charging Handle Variables
	float _Sqrd_ChargingHandlePerMoveOffset = 0;
	float _Sqrd_ChargingHandleLineLength = 0;
	float _CurrentChargingHandleMovePercentage = 0;

	//Firing
	float _TimeBetweenShots = 0;
	bool _bFiring = false;
	bool _bRoundInChamber = false;
	bool _bCanEjectRound = false;
	bool _bCanInsertRound = false;
	bool _bWasFiring = false;
	bool _bHoldEmptyPos = false;
	bool _bCanPlaySliderBackSound = false;
	bool _bCanQuickReleaseSlider = false;
	bool _bTriggerPressed = false;
	uint8 _BurstShotsLeft = 0;

	//Cached Info
	AGunBase* _GunBase;
	USkeletalMeshComponent* _GunSKMesh;

	int _CurrentFireModeSelected = 0;

	//ReloadSystem
	UReloadSystem* _ReloadSystem;

	FFireMode _CurrentFireMode;

};
