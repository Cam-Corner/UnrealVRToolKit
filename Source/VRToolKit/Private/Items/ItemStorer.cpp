// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemStorer.h"
#include "Components/SphereComponent.h"
#include "Items/VRItem.h"

// Sets default values
UItemStorer::UItemStorer()
{
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

bool UItemStorer::StoreItem(AVRItem* ItemToStore)
{
	if (!ItemToStore)
	{
		_ItemStored = NULL;
		return true;
	}
	else
	{
		if (_ItemStored)
			return false;

		_ItemStored = ItemToStore;
	}	

	return true;
}

bool UItemStorer::ItemSizeAllowed(EItemSize SizeAllowed)
{
	for (EItemSize IS : _ItemSizesAllowed)
	{
		if (IS == SizeAllowed)
			return true;
	}

	return false;
}


