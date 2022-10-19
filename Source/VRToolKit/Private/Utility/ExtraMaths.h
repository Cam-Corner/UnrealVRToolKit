// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


/**
 * 
 */
static class VRTOOLKIT_API ExtraMaths
{
public:

	/* returns the closest point on the line of VecA-VecB */
	static FVector PointProjectionOnLine(FVector VecA, FVector VecB, FVector Point, bool bClampValue = false)
	{
		FVector A = VecA;
		FVector B = VecB;
		FVector P = Point;

		FVector AP = P - A;
		FVector AB = B - A;
		FVector Result = A + FVector::DotProduct(AP, AB) / FVector::DotProduct(AB, AB) * AB;

		if (bClampValue)
		{
			/*FVector DirA = VecB - VecA;
			DirA.Normalize();
			
			FVector ResultDirA = Result - VecA;
			ResultDirA.Normalize();
			
			FVector DirB = VecA - VecB;
			DirB.Normalize();

			FVector ResultDirB = Result - VecB;
			ResultDirB.Normalize();

			if (ResultDirB != DirB)
				Result = VecB;
			else if (ResultDirA != DirA)
				Result = VecA;*/

			float DistOfLine = FVector::Dist(VecA, VecB);
			float DistToA = FVector::Dist(VecA, Result);
			float DistToB = FVector::Dist(VecB, Result);

			if (DistToA > DistOfLine || DistToB > DistOfLine)
			{
				if (DistToA < DistToB)
					Result = VecA;
				else
					Result = VecB;
			}
		}

		return Result;
	}

	/* returns the angle of the 2 vectors in degrees */
	static float GetAngleOfTwoVectors(const FVector& VecA, const FVector& VecB)
	{
		float MagA = VecA.Size();
		float MagB = VecB.Size();

		float DotProduct = FVector::DotProduct(VecA, VecB);

		float Angle = FMath::Acos(DotProduct / (MagA * MagB));

		return FMath::RadiansToDegrees(Angle);
	}

	/* returns the angle of the 2 vectors in degrees */
	static float GetSignedAngleOfTwoVectors(const FVector& From, const FVector& To, const FVector& RightVector)
	{
		float MagA = From.Size();
		float MagB = To.Size();

		float DotProduct = FVector::DotProduct(From, To);

		float Angle = FMath::Acos(DotProduct / (MagA * MagB));

		float AngleSign = FVector::DotProduct(RightVector, To);

		if (AngleSign > 0)
			AngleSign = 1;
		else
			AngleSign = -1;

		return AngleSign * FMath::RadiansToDegrees(Angle);
	}

	/* Normalize doesnt seem to be working correctly in UE5 so this functions corrects it */
	static void CorrectNormalizedVector(FVector& Vector)
	{
		float Size = 0;

		if (Vector.X < 0)
			Size += (Vector.X * -1);
		else
			Size += (Vector.X);

		if (Vector.Y < 0)
			Size += (Vector.Y * -1);
		else
			Size += (Vector.Y);

		if (Vector.Z < 0)
			Size += (Vector.Z * -1);
		else
			Size += (Vector.Z);

		if (Size == 0)
			Vector = FVector::ZeroVector;
		else
			Vector = Vector / Size;
	}
};
