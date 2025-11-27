// Rodney Torres All Rights Reserved

#pragma once

#include "CoreMinimal.h"
//include for manipulating out pins
#include "WarriorTypes/WarriorEnumTypes.h"

//we inherit from FPendingLatentAction which I don't know much about yet
class FWarriorCountDownAction : public FPendingLatentAction
{
public:
	//Init our variables properly with the function params and a init list (:). 
	FWarriorCountDownAction(float InTotalCountDownTime, float InUpdateInterval, float& InOutRemainingTime, EWarriorCountDownActionOutput& InCountDownOutput, FLatentActionInfo& LatentInfo)
	: bNeedToCancel(false)
	, TotalCountDownTime(InTotalCountDownTime)
	, UpdateInterval(InUpdateInterval)
	, OutRemainingTime(InOutRemainingTime)
	, CountDownOutput(InCountDownOutput)
	//We can get the next 3 init values from the latent info struct
	, ExecutionFunction(LatentInfo.ExecutionFunction)
	, OutputLink(LatentInfo.Linkage)
	, CallbackTarget(LatentInfo.CallbackTarget)
	, ElapsedInterval(0.f)
	, ElapsedTimeSinceStart(0.f)
	{
	}
	
	//Were overriding a function that gets called every tick
	virtual void UpdateOperation(FLatentResponse& Response) override;
	
	//Function for cancelling action inside our static function input checks using a bool variable
	void CancelAction();
	
private:
	//These variables we will initialize from our static function which we will do later
	bool bNeedToCancel;
	float TotalCountDownTime;
	float UpdateInterval;
	float& OutRemainingTime;
	//Here we need a variable to manipulate our out pins. For that we need a #include for our enum types
	EWarriorCountDownActionOutput& CountDownOutput;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	float ElapsedInterval;
	float ElapsedTimeSinceStart;
};