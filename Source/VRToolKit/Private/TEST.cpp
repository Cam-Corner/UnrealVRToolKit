// Fill out your copyright notice in the Description page of Project Settings.


#include "TEST.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Utility/ExtraMaths.h"

// Sets default values
ATEST::ATEST()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	_Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = _Root;

	_FingerStartPoint = CreateDefaultSubobject<UStaticMeshComponent>("FingerStartPoint");
	_FingerStartPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_FingerStartPoint->SetupAttachment(_Root);

	_ParmStartPoint = CreateDefaultSubobject<UStaticMeshComponent>("ParmStartPoint");
	_ParmStartPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_ParmStartPoint->SetupAttachment(_Root);
}

// Called when the game starts or when spawned
void ATEST::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATEST::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	FHitResult FingerHit;
	FHitResult ParmHit;
	FCollisionShape Shape;

	
	FCollisionQueryParams ColParams;
	ColParams.AddIgnoredComponent(_FingerStartPoint);
	ColParams.AddIgnoredComponent(_ParmStartPoint);

	//Finger Box
	{
		FVector3f BoxExtent(2.5f, 2.5f, 1.f);
		Shape.SetBox(BoxExtent);

		FVector Start = _FingerStartPoint->GetComponentLocation();
		FVector End = Start - FVector(0, 0, 5);

		bool Hit = GetWorld()->SweepSingleByChannel(FingerHit, Start, End, _FingerStartPoint->GetComponentQuat(),
			ECollisionChannel::ECC_WorldDynamic, Shape, ColParams);

		if (Hit)
			DrawDebugBox(GetWorld(), FingerHit.ImpactPoint, (FVector)BoxExtent, _FingerStartPoint->GetComponentQuat(), FColor::Red, false, .1f);
		else
			DrawDebugBox(GetWorld(), End, (FVector)BoxExtent, _FingerStartPoint->GetComponentQuat(), FColor::Green, false, .1f);


	}

	//Parm Box
	{
		FVector Start = _ParmStartPoint->GetComponentLocation();
		FVector ForwardNoZ = _ParmStartPoint->GetForwardVector();
		ForwardNoZ.Z = 0;
		ForwardNoZ.Normalize();
		FVector End = Start + (ForwardNoZ * 5);

		FVector3f BoxExtent(1.f, 2.5f, 5.f);
		Shape.SetBox(BoxExtent);

		bool Hit = GetWorld()->SweepSingleByChannel(ParmHit, Start, End, _ParmStartPoint->GetComponentQuat(),
			ECollisionChannel::ECC_WorldDynamic, Shape, ColParams);

		if (Hit)
			DrawDebugBox(GetWorld(), ParmHit.ImpactPoint, (FVector)BoxExtent, _ParmStartPoint->GetComponentQuat(), FColor::Red, false, .1f);
		else
			DrawDebugBox(GetWorld(), End, (FVector)BoxExtent, _ParmStartPoint->GetComponentQuat(), FColor::Green, false, .1f);

	}

	if (FingerHit.bBlockingHit && ParmHit.bBlockingHit)
	{
		float Angle = ExtraMaths::GetAngleOfTwoVectors(FingerHit.ImpactNormal, ParmHit.ImpactNormal);
		GEngine->AddOnScreenDebugMessage(44, 10.f, FColor::Red, "Grab Angle: " + FString::SanitizeFloat(Angle));

		DrawDebugLine(GetWorld(), FingerHit.ImpactPoint, FingerHit.ImpactPoint + (FingerHit.ImpactNormal * 20), FColor::Black);
		DrawDebugLine(GetWorld(), ParmHit.ImpactPoint, ParmHit.ImpactPoint + (ParmHit.ImpactNormal * 20), FColor::Black);
	}
}

