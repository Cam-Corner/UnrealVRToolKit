// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "ItemStorer.generated.h"

class USphereComponent;
class AVRItem;
enum EItemSize;

UCLASS(meta = (DisplayName = "Item Storer", BlueprintSpawnableComponent))
class VRTOOLKIT_API UItemStorer : public USphereComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UItemStorer();

public:
	bool StoreItem(AVRItem* ItemToStore);

	bool ItemSizeAllowed(EItemSize SizeAllowed);

	AVRItem* GetItemStored() { return _ItemStored; }
private:
	AVRItem* _ItemStored;

	UPROPERTY(EditAnywhere, Category = "Default Settings")
	TArray<TEnumAsByte<EItemSize>> _ItemSizesAllowed;

protected:

};
