#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "GameTimer.generated.h"

USTRUCT(BlueprintType)
struct FTime
{
	GENERATED_BODY()

public:
	float Seconds = 0;
	int Minutes = 0;
	int Hours = 0;

	static float GetTimeInSeconds(FTime Time)
	{
		return Time.Seconds + (Time.Minutes * 60) + (Time.Hours * 60 * 60);
	}

	static FTime GetSecondsToTime(float InSeconds)
	{
		FTime Time;
		
		//get remaining seconds
		float SecRemain = fmod(InSeconds, 60);
		Time.Seconds = SecRemain;
		
		//Get remaining minutes and hours
		SecRemain = InSeconds - SecRemain;
		int Minutes = SecRemain / 60;
		
		if (Minutes >= 60)
		{
			int MinsLeft = fmod(Minutes, 60);
			Time.Minutes = MinsLeft;

			int Hours = (Minutes - MinsLeft) / 60;
			Time.Hours = Hours;
		}
		else
		{
			Time.Minutes = Minutes;
			Time.Hours = 0;
		}

		return Time;
	}

	static FString GetTimeToString(FTime Time)
	{
		FString FinalString = "";
		bool bHoursLeft = false;
		bool bMinsLeft = false;

		if (Time.Hours > 0)
		{
			FinalString = FString::FromInt(Time.Hours) + ":";
			bHoursLeft = true;
		}

		if (Time.Minutes > 0)
		{
			if(bHoursLeft && Time.Minutes < 10)
				FinalString += "0" + FString::FromInt(Time.Minutes) + ":";
			else
				FinalString += FString::FromInt(Time.Minutes) + ":";
			
		}
		else if (bHoursLeft)
		{
			FinalString += "00:";
		}

		if(Time.Seconds < 10)
			FinalString += "0" + FString::SanitizeFloat(Time.Seconds);
		else
			FinalString += FString::SanitizeFloat(Time.Seconds);

		return FinalString;
		
	}
};

USTRUCT(BlueprintType)
struct FGameTimer
{
	GENERATED_BODY()

public:
	void SetTime(FTime InTime) { Timer = FTime::GetTimeInSeconds(InTime); } 

	void SetTime(float InTime) { Timer = InTime; } 

	/** Needs to be called to update the timer */
	void UpdateTimer(float DeltaTime)
	{
		bIsCountDownTimer ? Timer -= DeltaTime : Timer += DeltaTime;
	}

	FTime GetTime() { return FTime::GetSecondsToTime(Timer); }

	float GetTimeInSeconds() { return Timer; }

private:
	float Timer = 0; //everything done in seconds and converted to FTime when needed

	bool bIsCountDownTimer = false; //if true the timer counts down, if false the timer counts up
};