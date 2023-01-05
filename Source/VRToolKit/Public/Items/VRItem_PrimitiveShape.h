// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/VRItem.h"
#include "VRItem_PrimitiveShape.generated.h"

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API AVRItem_PrimitiveShape : public AVRItem
{
	GENERATED_BODY()
public:
	AVRItem_PrimitiveShape();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UBoxComponent* _RootPhysicsBox;
	
};
