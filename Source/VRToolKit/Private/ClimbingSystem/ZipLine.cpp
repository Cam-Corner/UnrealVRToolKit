// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingSystem/ZipLine.h"
#include "Components/BillboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ClimbingSystem/EnvironmentGrabComponent.h"
#include "Utility/ExtraMaths.h"

// Sets default values
AZipLine::AZipLine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_ZiplineStart = CreateDefaultSubobject<UBillboardComponent>("ZipLineStart");
	RootComponent = _ZiplineStart;

	_ZiplineEnd = CreateDefaultSubobject<UBillboardComponent>("ZipLineEnd");
	_ZiplineEnd->SetupAttachment(RootComponent);

	_LineMesh = CreateDefaultSubobject<UStaticMeshComponent>("LineMesh");
	_LineMesh->SetupAttachment(RootComponent);

	_HandleMesh = CreateDefaultSubobject<UStaticMeshComponent>("HandleMesh");
	_HandleMesh->SetupAttachment(RootComponent);

	_EGrabComp = CreateDefaultSubobject<UEnvironmentGrabComponent>("EGrabComp");
	_EGrabComp->SetupAttachment(_HandleMesh);
}

// Called when the game starts or when spawned
void AZipLine::BeginPlay()
{
	Super::BeginPlay();
	
	if (_EGrabComp)
		_EGrabComp->_ComponentTrigger.AddDynamic(this, &AZipLine::TriggerPressed);
}

// Called every frame
void AZipLine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateZiplineSlide();
}

void AZipLine::TriggerPressed(float Value)
{
	if (Value > .5f)
		_bTriggerPressed = true;
	else
		_bTriggerPressed = false;

}


void AZipLine::UpdateZiplineSlide()
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	float Angle = 1; 
	
	if(_bAngleEffectsSpeed)
		Angle = ExtraMaths::GetAngleOfTwoVectors(FVector::UpVector, _ZiplineDir);

	if (_bTriggerPressed)
	{
		_CurrentVelocity += (_ZipLineAcceleration * Angle) * DeltaTime;
	}
	else
	{
		_CurrentVelocity -= (_ZipLineSpeedDecceleration * Angle) * DeltaTime;
	}

	_CurrentVelocity = FMath::Clamp(_CurrentVelocity, 0, _MaxZipLineSpeed);
	
	if (!_HandleMesh)
		return;

	_HandleMesh->AddWorldOffset(_ZiplineDir * _CurrentVelocity * DeltaTime);

	if (_HandleMesh->GetComponentLocation().Z < _ZiplineEnd->GetComponentLocation().Z)
	{
		_HandleMesh->SetWorldLocation(_ZiplineEnd->GetComponentLocation());
	}
}

void AZipLine::UpdateComponent()
{
	if (!_ZiplineStart || !_ZiplineEnd || !_LineMesh)
		return;

	FVector StartLoc = _ZiplineStart->GetComponentLocation();
	FVector EndLoc = _ZiplineEnd->GetComponentLocation();

	FVector Offset = EndLoc - StartLoc;

	//put in correct rotation
	FVector DesiredDir = Offset.GetSafeNormal();
	_ZiplineDir = DesiredDir;

	FRotator Rot = DesiredDir.ToOrientationRotator();

	_LineMesh->SetWorldRotation(Rot);
	_LineMesh->SetWorldLocation(StartLoc + (Offset / 2));
	float FinalXScale = FVector::Distance(StartLoc, EndLoc) / _DefaultLineLength;
	FVector FinalScale = _LineMesh->GetComponentScale();
	FinalScale.X = FinalXScale;
	_LineMesh->SetWorldScale3D(FinalScale);
}

