// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Guns/GunAnimInstance.h"

UGunAnimInstance::UGunAnimInstance()
{
}

void UGunAnimInstance::AddBone(FName BoneName, uint8 BoneIndex)
{
	FAnimIBoneTransforms NewBoneT;
	NewBoneT._BoneIndex = BoneIndex;
	NewBoneT._BoneName = BoneName;

	_BoneTransforms.Add(NewBoneT);
}

void UGunAnimInstance::SetBoneInfo(float Info, uint8 BoneIndex)
{
	for (int i = 0; i < _BoneTransforms.Num(); i++)
	{
		if (_BoneTransforms[i]._BoneIndex == BoneIndex)
		{
			_BoneTransforms[i]._BoneInfo = Info;
		}
	}
}
