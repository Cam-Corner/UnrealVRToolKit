// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/VRItem.h"
#include "VRItem_StaticMesh.generated.h"

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API AVRItem_StaticMesh : public AVRItem
{
	GENERATED_BODY()

public:
	AVRItem_StaticMesh();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* _ItemMesh;

};
