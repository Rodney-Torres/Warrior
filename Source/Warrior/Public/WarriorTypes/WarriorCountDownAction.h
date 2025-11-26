// Rodney Torres All Rights Reserved

#pragma once

#include "CoreMinimal.h"
//include for manipulating out pins
#include "WarriorTypes/WarriorEnumTypes.h"

//we inherit from FPendingLatentAction which I don't know much about yet
class FWarriorCountDownAction : public FPendingLatentAction
{
public:
	//This now needs to init our variables properly. It needs function inputs first for that.
	//& reference symbol is used to manipulate the data from our static function
	//Now with all of these function inputs we can init our member variables and were going to do that through our init list.
	//The init list is new to me but before {} we put a : and then we put our member variable name followed by () with the value we want to init it with.
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
	
	//Function for cancelling action inside our static function input checks
	void CancelAction();
	
private:
	//These variables we will initialize from our static function which we will do later
	//Bool to cancel our latent action
	bool bNeedToCancel;
	float TotalCountDownTime;
	float UpdateInterval;
	//Variable for notifying our BP the out remaining time. & Symbol is used again for outputs. To do that we need to manipulate the out pins of our static function
	//For that were gonna need our WarriorFunctionLibraries include
	float& OutRemainingTime;
	//Use this variable to decide which variable to fire in our countdown node
	EWarriorCountDownActionOutput& CountDownOutput;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	float ElapsedInterval;
	float ElapsedTimeSinceStart;
};