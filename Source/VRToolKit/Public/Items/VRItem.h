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
protected:
	UPROPERTY(EditAnywhere);
	UPhysicsHandlerComponent* _PHC;

};
