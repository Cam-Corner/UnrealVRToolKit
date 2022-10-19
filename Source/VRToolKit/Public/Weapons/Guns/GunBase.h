// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "GunBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTriggerPressed);

class UReloadSystem;

USTRUCT(BlueprintType)
struct FGunTrigger
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FName _TriggerBoneName = "b_gun_trigger";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
		float _PressedValue = 0;
	
	FTriggerPressed _TriggerPressed;

	FTriggerPressed _TriggerReleased;

	bool _CanPress = false;
};

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API AGunBase : public AWeaponBase
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AGunBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FGunTrigger* GetTriggerByIndex(int Index);

	UFUNCTION(BlueprintCallable)
	void SetReloadSystem(UReloadSystem* ReloadSystem);

	UFUNCTION(BlueprintCallable)
	void AddRecoilOffset();

	FTriggerPressed _BottomButtonPressed;
	FTriggerPressed _TopButtonPressed;
protected:
	UPROPERTY(EditAnywhere, Category = "Trigger", BlueprintReadWrite)
	FGunTrigger _GunTrigger;
		//TArray<FGunTrigger> _GunTriggers;

	UReloadSystem* _ReloadSystem;

	UFUNCTION()
		void TriggerPressed(float Value);

	UFUNCTION()
		void BottomButtonPressed(bool bPressed);

	UFUNCTION()
		void TopButtonPressed(bool bPressed);

	virtual void EditWeaponOffset(float DeltaTime) override;

	virtual void HandleRecoilRecover();

	virtual void HandleRecoil();

	void ConsistantResetToValue(float& CurrentValue, float GotoValue, float Alpha);
	void ConsistantResetToValue(FVector& CurrentValue, FVector GotoValue, float Alpha);

	bool _bInRecoilState = false;
	bool _bCanRecoverRecoil = false;
	FQuat _CurrentRecoilQuat = FQuat(FVector::ForwardVector, 0);

	//Recoil
	float _CurrentVerticalRecoil = 0;
	float _CurrentHorizontalRecoil = 0;
	FVector _CurrentKickBack = FVector::ZeroVector;

	float _GotoVerticalRecoil = 0;
	float _GotoHorizontalRecoil = 0;
	FVector _GotoKickBack = FVector::ZeroVector;

	float _bCanRecoverVerticalRecoil = false;
	float _bCanRecoverHorizontalRecoil = false;
	float _bCanRecoverKickBackRecoil = false;
private:
	UPROPERTY(EditAnywhere, Category = "GunRecoil")
		bool _bRandomHorizontalRecoil = true;

	UPROPERTY(EditAnywhere, Category = "GunRecoil", meta = (EditCondition = "_bRandomHorizontalRecoil"))
		float _MinHorizontalOffset = 5.f;

	UPROPERTY(EditAnywhere, Category = "GunRecoil")
		float _MaxHorizontalOffset = 5.f;

	UPROPERTY(EditAnywhere, Category = "GunRecoil")
		bool _bRandomVerticalRecoil = false;

	UPROPERTY(EditAnywhere, Category = "GunRecoil", meta = (EditCondition = "_bRandomVerticalRecoil"))
		float _MinVerticalOffset = -5.f;

	UPROPERTY(EditAnywhere, Category = "GunRecoil")
		float _MaxVerticalOffset = 5.f;

	UPROPERTY(EditAnywhere, Category = "GunRecoil")
		bool _bRandomKickBack = false;

	UPROPERTY(EditAnywhere, Category = "GunRecoil", meta = (EditCondition = "_bRandomKickBack"))
		FVector _MinKickBackForce = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "GunRecoil")
		FVector _MaxKickBackForce = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "GunRecoil")
		float _RecoilFollowTime = .25f;

	UPROPERTY(EditAnywhere, Category = "GunRecoil")
	FVector _OffsetToWrist = FVector(5, 0, 0);

	UPROPERTY(EditAnywhere, Category = "GunRecoil")
		float _RecoverTime = .25f;

};
