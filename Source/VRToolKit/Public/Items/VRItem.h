// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utility/PIDControllers.h"
#include "GameFramework/Actor.h"
#include "VRItem.generated.h"

class UItemGrabComponent;
class AVRHand;
class UStaticMeshComponent;
class UPhysicsHandlerComponent;
class UItemStorer;

UENUM()
enum EItemSize
{
	EIS_ItemSmall		UMETA(MetaTag1="ItemSmall"),
	EIS_ItemMedium		UMETA(MetaTag1="ItemMedium"),
	EIS_ItemLarge		UMETA(MetaTag1="ItemLarge"),
};

UCLASS()
class VRTOOLKIT_API AVRItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVRItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ForceDrop(bool bDestroyAfter = false);

	bool IsBeingHeld();

	virtual UClass* GetItemMagazine() { return NULL; }

	UItemGrabComponent* GetGrabPoint() { return _MainGrabComponent; }

	UFUNCTION(BlueprintCallable)
	void BP_SetRootComponent(USceneComponent* NewRootComp);

protected:
	UFUNCTION()
	virtual void MainGrabPointGrabbed(AVRHand* Hand);

	UFUNCTION()
	virtual void MainGrabPointReleased(AVRHand* Hand);

	UPROPERTY(VisibleAnywhere)
	UItemGrabComponent* _MainGrabComponent;

	UPROPERTY(EditAnywhere, Category = "Spring")
	FPDController3D _LocPD;

	UPROPERTY(EditAnywhere, Category = "Spring")
	FQuatPDController _RotPD;

	UPROPERTY()
	class UPrimitiveComponent* _RootPrimitiveComponent;

	void SetupItemRootComponent();

	UFUNCTION()
		void OnItemOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnItemOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent)
		void ComponentGrabbed(bool bGrabbed, bool bRightHand);

	virtual void OverlappedItemStorer(UItemStorer* ItemStorer);
protected:
	UPROPERTY(EditAnywhere);
	UPhysicsHandlerComponent* _PHC;

	UPROPERTY()
	UItemStorer* _StoredIn = NULL;

	UPROPERTY()
	TArray<UItemStorer*> _ItemStorers;

	UPROPERTY(EditAnywhere, Category = "Item Defaults")
	TEnumAsByte<EItemSize> _ItemSize;

	UPROPERTY(EditAnywhere, Category = "Default Settings")
	bool _bKeepWeaponStoredOnItemStorer = false;

	bool _bInStorer = false;
};
