// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/VRItem_StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Items/ItemGrabComponent.h"

AVRItem_StaticMesh::AVRItem_StaticMesh()
{
	_ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	SetRootComponent(_ItemMesh);

	_MainGrabComponent->SetupAttachment(_ItemMesh);
}

