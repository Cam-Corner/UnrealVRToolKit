// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZipLine.generated.h"

class UBillboardComponent;
class UStaticMeshComponent;
class UEnvironmentGrabComponent;

UCLASS()
class VRTOOLKIT_API AZipLine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZipLine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere)
	UBillboardComponent* _ZiplineStart;

	UPROPERTY(VisibleAnywhere)
	UBillboardComponent* _ZiplineEnd;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* _LineMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* _HandleMesh;

	UPROPERTY(VisibleAnywhere)
	UEnvironmentGrabComponent* _EGrabComp;

	/* The defautl length of the line mesh (in unreal units)*/
	UPROPERTY(EditAnywhere)
	float _DefaultLineLength = 100;

	UPROPERTY(EditAnywhere)
	float _MaxZipLineSpeed = 1000;

	UPROPERTY(EditAnywhere)
	float _ZipLineAcceleration = 50;

	UPROPERTY(EditAnywhere)
	float _ZipLineSpeedDecceleration = 25;

	UPROPERTY(EditAnywhere)
	bool  _bAngleEffectsSpeed = true;

	float _CurrentVelocity = 0;

	UPROPERTY()
	FVector _ZiplineDir = FVector::ZeroVector;
protected:
	UFUNCTION()
	void TriggerPressed(float Value);

	void UpdateZiplineSlide();

	bool _bTriggerPressed;

#if WITH_EDITORONLY_DATA
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Buttons")
		void UpdateComponent();
#endif
};
