// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/VRItem_SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Items/ItemGrabComponent.h"

AVRItem_SkeletalMesh::AVRItem_SkeletalMesh()
{
	_ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ItemMesh");
	SetRootComponent(_ItemMesh);

	_MainGrabComponent->SetupAttachment(_ItemMesh);
}

