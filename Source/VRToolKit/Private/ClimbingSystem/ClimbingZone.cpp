// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingSystem/ClimbingZone.h"
#include "Components/BillboardComponent.h"
#include "Components/SceneComponent.h"
#include "ClimbingSystem/EnvironmentGrabComponent.h"
#include "Utility/ExtraMaths.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

// Sets default values
AClimbingZone::AClimbingZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_RootComp = CreateDefaultSubobject<USceneComponent>("RootComp");
	SetRootComponent(_RootComp);

	/*_ZoneStart = CreateDefaultSubobject<UBillboardComponent>("ZoneStart");
	_ZoneStart->SetupAttachment(_RootComp);
	_ZoneStart->bHiddenInGame = false;

	_ZoneEnd = CreateDefaultSubobject<UBillboardComponent>("ZoneEnd");
	_ZoneEnd->SetupAttachment(_RootComp);
	_ZoneEnd->bHiddenInGame = false;

	_GrabComp = CreateDefaultSubobject<UEnvironmentGrabComponent>("GrabComp");
	_GrabComp->SetupAttachment(_RootComp);
	_GrabComp->bHiddenInGame = false;*/

#if WITH_EDITORONLY_DATA
	static ConstructorHelpers::FObjectFinder<UMaterial> MatClimb(TEXT("/VRToolKit/DontDelete/DebugMaterials/m_ClimbingBox"));
	if (MatClimb.Succeeded())
	{
		_ClimbShowMat = MatClimb.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> MatSelectedPoint(TEXT("/VRToolKit/DontDelete/DebugMaterials/m_SelectedClimbingPoint"));
	if (MatSelectedPoint.Succeeded())
	{
		_ClimbPointSelectedMat = MatSelectedPoint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> MatPoint(TEXT("/VRToolKit/DontDelete/DebugMaterials/m_ClimbingPoint"));
	if (MatPoint.Succeeded())
	{
		_ClimbPointMat = MatPoint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cube"));
	if (Mesh.Succeeded())
	{
		_MeshToUse = Mesh.Object;
	}
#endif
}

// Called when the game starts or when spawned
void AClimbingZone::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AClimbingZone::ShouldTickIfViewportsOnly() const
{
	return true;
}

// Called every frame
void AClimbingZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITORONLY_DATA
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor && IsSelectedInEditor())
	{
		bool bChange = false;
		for (int i = 0; i < _TempPoints.Num(); i++)
		{
			if (i >= _Points.Num())
			{
				bChange = true;
				break;
			}

			if (_TempPoints[i]->GetComponentLocation() != _Points[i])
			{
				bChange = true;
				break;
			}
		}

		//DrawPoints();	
		if (_Points.Num() != _TempPoints.Num() || bChange)
		{
			_Points.Empty();

			for (UStaticMeshComponent* M : _TempPoints)
			{
				_Points.Add(M->GetComponentLocation());
			}
			 
			ReconstructClimbingZone();
		}		
	}
#endif
}
#if WITH_EDITORONLY_DATA

void AClimbingZone::SelectNextPoint()
{
	if(_SelectedPointIndex < _TempPoints.Num())
		_TempPoints[_SelectedPointIndex]->SetMaterial(0, _ClimbPointMat);

	_SelectedPointIndex++;
	
	if (_SelectedPointIndex > _TempPoints.Num() - 1)
		_SelectedPointIndex = 0;

	if (_SelectedPointIndex < _TempPoints.Num())
		_TempPoints[_SelectedPointIndex]->SetMaterial(0, _ClimbPointSelectedMat);
}

void AClimbingZone::SelectPreviousPoint()
{
	if (_SelectedPointIndex < _TempPoints.Num())
		_TempPoints[_SelectedPointIndex]->SetMaterial(0, _ClimbPointMat);

	if (_SelectedPointIndex > 0)
		_SelectedPointIndex--;
	else
		_SelectedPointIndex = _TempPoints.Num() - 1;

	if (_SelectedPointIndex < _TempPoints.Num())
		_TempPoints[_SelectedPointIndex]->SetMaterial(0, _ClimbPointSelectedMat);
}

void AClimbingZone::AddPoint()
{
	CreatePointComponent();
}

void AClimbingZone::RemovePoint()
{
	_TempPoints[_SelectedPointIndex]->DestroyComponent();
	_TempPoints.RemoveAt(_SelectedPointIndex);
	SelectNextPoint();
}
#endif

void AClimbingZone::ReconstructClimbingZone()
{
	for (UEnvironmentGrabComponent* EGC : _GrabPoints)
	{
		EGC->DestroyComponent();
	}

	_GrabPoints.Empty();

	for (UStaticMeshComponent* SMC : _VisibleClimbMesh)
	{
		SMC->DestroyComponent();
	}

	_VisibleClimbMesh.Empty();

	for (int i = 0; i < _Points.Num(); i++)
	{
		if (i > 0)
		{
			FVector ZoneStartLoc = _Points[i - 1];
			FVector ZoneEndLoc = _Points[i];

			FVector Offset = ZoneEndLoc - ZoneStartLoc;

			//put in correct rotation
			FVector DesiredDir = ZoneEndLoc - ZoneStartLoc;
			DesiredDir.Normalize();
			FRotator Rot = DesiredDir.ToOrientationRotator();

			CreateClimbingZoneComponent(ZoneStartLoc + (Offset / 2), Rot, FVector(FVector::Distance(ZoneEndLoc, ZoneStartLoc) / 2, 4.f, 2.f));
		}
	}
}

void AClimbingZone::DrawPoints()
{	
	if (!GetWorld())
		return;

	/*float Index = 0;

	for (UStaticMeshComponent* MC : _Points)
	{
		FVector Loc = MC->GetComponentLocation();

		if(Index == _SelectedPointIndex)
			DrawDebugSphere(GetWorld(), Loc, 15.f, 32, FColor::Red, false, .1f);
		else
			DrawDebugSphere(GetWorld(), Loc, 15.f, 32, FColor::Black, false, .1f);

		Index++;
	}*/

	/*for (UEnvironmentGrabComponent* GC : _GrabPoints)
	{
		FTransform Loc = GC->GetComponentTransform();
		
		DrawDebugSolidBox(GetWorld(), Loc.GetLocation(), GC->GetScaledBoxExtent(), Loc.GetRotation(), FColor::Blue, false, .1f);
	}*/
}

void AClimbingZone::OnConstruction(const FTransform& Transform)
{
	ReconstructClimbingZone();
}

void AClimbingZone::CreatePointComponent()
{
	FName Name = "NewPoint";
	FVector CompLoc = GetActorLocation();

	if (_TempPoints.Num() > 0)
		CompLoc = _TempPoints[_SelectedPointIndex]->GetComponentLocation() + FVector(5, 5, 5);

	UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>(this);

	if (!NewComp)
		return;

	NewComp->RegisterComponent();
	NewComp->SetWorldLocation(CompLoc);
	NewComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	NewComp->bHiddenInGame = true;
	NewComp->SetWorldScale3D(FVector(.1f, .1f, .1f));
	NewComp->SetStaticMesh(_MeshToUse);
	NewComp->SetMaterial(0, _ClimbPointMat);

	if (_SelectedPointIndex == _TempPoints.Num() - 1 || _TempPoints.Num() == 0)
	{
		_TempPoints.Add(NewComp);
	}
	else
	{
		_TempPoints.Add(NULL);

		for (int i = 0; i < _TempPoints.Num(); i++)
		{
			if (i > _SelectedPointIndex + 1 && i - 1 > 0)
			{
				_TempPoints[i] = _TempPoints[i - 1];
			}
		}

		_TempPoints[_SelectedPointIndex + 1] = NewComp;
	}

	SelectNextPoint();
}

void AClimbingZone::CreateClimbingZoneComponent(FVector Location, FRotator Rotation, FVector BoxExtent)
{
	UEnvironmentGrabComponent* NewComp = NewObject<UEnvironmentGrabComponent>(this);
	NewComp->bHiddenInGame = true;
	NewComp->SetVisibility(false);
	NewComp->SetBoxExtent(BoxExtent);

	if (!NewComp)
		return;

	NewComp->RegisterComponent();
	NewComp->SetWorldLocation(Location);
	NewComp->SetWorldRotation(Rotation);
	//NewComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

	_GrabPoints.Add(NewComp);

#if WITH_EDITORONLY_DATA
	FVector Size = BoxExtent / 50;

	UStaticMeshComponent* NewMeshComp = NewObject<UStaticMeshComponent>(this);
	//NewComp->bHiddenInGame = true;
	NewMeshComp->SetVisibility(true);
	NewMeshComp->SetWorldScale3D(Size);
	NewMeshComp->SetStaticMesh(_MeshToUse);
	NewMeshComp->SetMaterial(0, _ClimbShowMat);

	if (!NewMeshComp)
		return;

	NewMeshComp->RegisterComponent();
	NewMeshComp->SetWorldLocation(Location);
	NewMeshComp->SetWorldRotation(Rotation);

	_VisibleClimbMesh.Add(NewMeshComp);
#endif
}

