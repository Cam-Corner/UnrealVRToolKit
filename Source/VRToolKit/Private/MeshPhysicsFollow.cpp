// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshPhysicsFollow.h"
#include "Components/StaticMeshComponent.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
AMeshPhysicsFollow::AMeshPhysicsFollow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_FollowComp = CreateDefaultSubobject<USceneComponent>("FollowComp");
	RootComponent = _FollowComp;

	_STM_PhysicsMesh = CreateDefaultSubobject<UStaticMeshComponent>("STM_PhysicsMesh");
	_STM_PhysicsMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_STM_PhysicsMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	_STM_PhysicsMesh->SetupAttachment(_FollowComp);

	_SKM_PhysicsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SKM_PhysicsMesh");
	_SKM_PhysicsMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_SKM_PhysicsMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	_SKM_PhysicsMesh->SetupAttachment(_FollowComp);

	_OnCalculateCustomPhysics.BindUObject(this, &AMeshPhysicsFollow::CustomPhysics);

	_PHC = CreateDefaultSubobject<UPhysicsHandlerComponent>("PHC");
}

// Called when the game starts or when spawned
void AMeshPhysicsFollow::BeginPlay()
{
	Super::BeginPlay();

	_PHC->SetTargetObject(_FollowComp);
	_PHC->SetMatchTargetAuthorityType(EAuthorityType::EAT_Client);
//	_PHC->SetMatchTarget(true);
	_PHC->SetTargetType(ETargetType::ETT_SetObject);
	_PHC->SetPlayerControllerOwner(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	if(_bUseSkeletalMeshes)
		_PHC->SetPhysicsObject(_SKM_PhysicsMesh);
	else
		_PHC->SetPhysicsObject(_STM_PhysicsMesh);

}

// Called every frame
void AMeshPhysicsFollow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

float AMeshPhysicsFollow::AngleDifference(float A, float B)
{
	float Result = (A - B) + 540;
	Result = (int)Result % 360;
	Result -= 180;
	return  Result;
}

void AMeshPhysicsFollow::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	
}

FVector AMeshPhysicsFollow::NewQuatPID(FQuat From, FQuat To, FVector AVel)
{

	
	return FVector::ZeroVector;// P + D;
}

