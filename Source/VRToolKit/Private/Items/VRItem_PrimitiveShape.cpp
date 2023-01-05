// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/VRItem_PrimitiveShape.h"
#include "Components/BoxComponent.h"
#include "Items/ItemGrabComponent.h"

AVRItem_PrimitiveShape::AVRItem_PrimitiveShape()
{
	_RootPhysicsBox = CreateDefaultSubobject<UBoxComponent>("RootPhysicsBox");
	SetRootComponent(_RootPhysicsBox);

	_MainGrabComponent->SetupAttachment(_RootPhysicsBox);
}
