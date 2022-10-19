// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Guns/GunMagazine.h"
#include "Items/VRGrabComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ActorComponents/PhysicsHandlerComponent.h"
#include "Player/VRHand.h"
#include "MotionControllerComponent.h"
#include "DrawDebugHelpers.h"


// Sets default values
AGunMagazine::AGunMagazine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

