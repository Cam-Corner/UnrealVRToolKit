// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/VRItem.h"
#include "VRItem_SkeletalMesh.generated.h"

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API AVRItem_SkeletalMesh : public AVRItem
{
	GENERATED_BODY()
public:
	AVRItem_SkeletalMesh();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class USkeletalMeshComponent* _ItemMesh;
};
