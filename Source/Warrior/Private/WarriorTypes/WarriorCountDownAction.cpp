// Rodney Torres All Rights Reserved


#include "WarriorTypes/WarriorCountDownAction.h"

//Inside here we don't need to call the parent version since it's not doing anything
void FWarriorCountDownAction::UpdateOperation(FLatentResponse& Response)
{
	//If this is true we end the action immediately. To do that we need to trigger the output pin cancelled on our node
	if (bNeedToCancel)
	{
		//This is how we can trigger the output pin and after this we should end our action with Response.FinishAndTriggerIf()
		CountDownOutput = EWarriorCountDownActionOutput::Cancelled;
		
		//This ends our action
		//true since we already checked it. We pass in our execution function name, output link and callback target from our init list then we return early.
		Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		
		return;
	}
	
	//So if we get passed the first if we check our elapsed time to see if its greater than equal to our total countdown time. If yes our countdown action has completed.
	//If this is true we should trigger our outpin pin completed
	if (ElapsedTimeSinceStart >= TotalCountDownTime)
	{
		CountDownOutput = EWarriorCountDownActionOutput::Completed;
		
		//This ends our action
		Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		
		return;
	}
	
	//So if we get passed the second if check then we need to update it. If the if is the case we need to accumulate it
	if (ElapsedInterval < UpdateInterval)
	{
		//We accumulate the elapsed time and this Response.ElapsedTime is like delta time inside a tick function
		ElapsedInterval += Response.ElapsedTime();
	}
	//In here if our elapsed interval is greater than we should trigger our update pin
	else
	{
		// So if its greater than were going to add UpdateInterval to our ElapsedTimeSinceStart and in the else case we should update every frame so add the Response.ElapsedTime()
		ElapsedTimeSinceStart += UpdateInterval > 0.f? UpdateInterval : Response.ElapsedTime();
		//Next calc is our OutRemainingTime.
		OutRemainingTime = TotalCountDownTime - ElapsedTimeSinceStart;
		//Then we can trigger our update pin
		CountDownOutput = EWarriorCountDownActionOutput::Updated;
		//Then we need to TriggerLink since we are not ending this action yet
		Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
		//The last thing is setting our elapsed interval to 0.f
		ElapsedInterval = 0.f;
	}
}

//Here we just need to set our variable bNeedToCancel to true
void FWarriorCountDownAction::CancelAction()
{
	bNeedToCancel = true;
}
