// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitBoxComponent.h"

UHitBoxComponent::UHitBoxComponent()
{
}

void UHitBoxComponent::HitBoxHit(float Damage)
{
	_HitBoxHit.Broadcast(Damage * _DamageMultiplier);
}
