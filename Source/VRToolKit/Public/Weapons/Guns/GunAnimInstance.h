// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GunAnimInstance.generated.h"


USTRUCT(BlueprintType)
struct FAnimIBoneTransforms
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneTransforms")
	FName _BoneName = "b_bone_name";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneTransforms")
	uint8 _BoneIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneTransforms")
	float _BoneInfo = 0.f;
};

/**
 * 
 */
UCLASS()
class VRTOOLKIT_API UGunAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UGunAnimInstance();

	void AddBone(FName BoneName, uint8 BoneIndex);
	void SetBoneInfo(float Info, uint8 BoneIndex);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneTransforms")
		TArray<FAnimIBoneTransforms> _BoneTransforms;

};
