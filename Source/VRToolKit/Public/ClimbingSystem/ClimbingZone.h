// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClimbingZone.generated.h"

class UEnvironmentGrabComponent;
class UBillboardComponent;
class USceneComponent;

UCLASS()
class VRTOOLKIT_API AClimbingZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AClimbingZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual bool ShouldTickIfViewportsOnly() const override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
#if WITH_EDITORONLY_DATA
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Buttons")
	void SelectNextPoint();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Buttons")
	void SelectPreviousPoint();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Buttons")
	void AddPoint();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Buttons")
	void RemovePoint();
#endif

	void ReconstructClimbingZone();

	void DrawPoints();

	virtual void OnConstruction(const FTransform& Transform) override;
protected:
	USceneComponent* _RootComp;

private:
	UPROPERTY()
	TArray<FVector> _Points;
		
	UPROPERTY()
	TArray<UEnvironmentGrabComponent*> _GrabPoints;
	
	uint8 _SelectedPointIndex = 0;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TArray<class UStaticMeshComponent*> _TempPoints;

	UPROPERTY()
	TArray<class UStaticMeshComponent*> _VisibleClimbMesh;

	class UStaticMesh* _MeshToUse;
	class UMaterial* _ClimbShowMat;
	class UMaterial* _ClimbPointMat;
	class UMaterial* _ClimbPointSelectedMat;
#endif

	void CreatePointComponent();
	void CreateClimbingZoneComponent(FVector Location, FRotator Rotation, FVector BoxExtent);
};
