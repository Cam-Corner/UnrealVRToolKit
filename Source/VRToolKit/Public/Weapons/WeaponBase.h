// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/VRItem_SkeletalMesh.h"
#include "WeaponBase.generated.h"

class USkeletalMeshComponent;
class UItemGrabComponent;
class UPhysicsHandlerComponent;

UCLASS()
class VRTOOLKIT_API AWeaponBase : public AVRItem_SkeletalMesh
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:
	/*UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon Skeletal Mesh")
	USkeletalMeshComponent* _WeaponSKM;*/

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon Skeletal Mesh")
	USkeletalMeshComponent* _HandGrabPose;

	virtual void EditWeaponOffset(float DeltaTime);
protected:
	virtual void MainGrabPointGrabbed(AVRHand* Hand) override;

	virtual void MainGrabPointReleased(AVRHand* Hand) override;
};
