#pragma once

#include "CoreMinimal.h"
#include "WeaponHelpers.generated.h"

UENUM(BlueprintType)
enum EWeaponSize
{
	EWS_Small		UMETA(DisplayName = "Small Weapons"),
	EWS_Medium		UMETA(DisplayName = "Medium Weapons"),
	EWS_Large		UMETA(DisplayName = "Large Weapons"),
};

