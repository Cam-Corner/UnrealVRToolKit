// Fill out your copyright notice in the Description page of Project Settings.


#include "Switches/SwitchReceiver.h"
#include "Switches/SwitchBase.h"

// Sets default values for this component's properties
USwitchReceiver::USwitchReceiver()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USwitchReceiver::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	if (_Switch)
	{
		_Switch->_SwitchEnabled.AddDynamic(this, &USwitchReceiver::SwitchEnabled);
		//_Switch->_SwitchEnabled.AddDynamic(this, &USwitchReceiver::SwitchEnabled_Blueprint);
	}
}


// Called every frame
void USwitchReceiver::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USwitchReceiver::SwitchEnabled(bool bEnabled)
{
	SwitchEnabled_Blueprint(bEnabled);
}

void USwitchReceiver::SwitchEnabled_Blueprint_Implementation(bool bEnabled)
{
}

