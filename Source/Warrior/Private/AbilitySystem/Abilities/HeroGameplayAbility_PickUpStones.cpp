// Rodney Torres All Rights Reserved


#include "AbilitySystem/Abilities/HeroGameplayAbility_PickUpStones.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Items/PickUps/WarriorStoneBase.h"

void UHeroGameplayAbility_PickUpStones::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UHeroGameplayAbility_PickUpStones::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//Perform a box trace to gethold of all the available stones on the ground
void UHeroGameplayAbility_PickUpStones::CollectStones()
{
	//Clear our collected stones array first
	CollectedStones.Empty();
	
	//OutHits new variable
	TArray<FHitResult> TraceHits;
	
	//needed a #include and need to qualify scope here. We use BoxTraceMultiForObjects and it needs several inputs
	//1.WorldContextObject, 2.Start, 3.End, 4.HalfSize, 5.Rotation, 6.ObjectTypes, 7.bTraceComplex, 8.IgnoredActors, 9.OutHits, 10.DrawDebugType, 11.DebugColor, 12.DebugThickness, 13.bIgnoreSelf
	UKismetSystemLibrary::BoxTraceMultiForObjects(
	GetHeroCharacterFromActorInfo(), //WorldContextObject
	GetHeroCharacterFromActorInfo()->GetActorLocation(), //Start location
	GetHeroCharacterFromActorInfo()->GetActorLocation() + -GetHeroCharacterFromActorInfo()->GetActorUpVector() * BoxTraceDistance, //Trace downwards to get end
	TraceBoxSize / 2.f, //divided by 2 to convert this into a half size
	(-GetHeroCharacterFromActorInfo()->GetActorUpVector()).ToOrientationRotator(), //Need our box to face down
	StoneTraceChannel, //Object types were tracing for
	false, // trace complex bool (dont know what this does)
	TArray<AActor*>(), //Actors to ignore we fill in a empty TArray (dont know what this does)
	bDrawDebugShape? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, //DrawDebugType were gonna create a bool to toggle this
	TraceHits, //OutHits which we created a new variable for it above this function
	true //bIgnoreSelf (dont know what this does)
	);
	
	//We loop through the array
	for (const FHitResult& TraceHit : TraceHits)
	{
		//Check if we hit a stone
		if (AWarriorStoneBase* FoundStone = Cast<AWarriorStoneBase>(TraceHit.GetActor()))
		{
			//store this FoundStone inside a array called CollectedStones
			CollectedStones.AddUnique(FoundStone);
		}
	}

	//If this array is empty there are no more stones in the level
	if (CollectedStones.IsEmpty())
	{
		//We cancel the ability
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
	}
}

void UHeroGameplayAbility_PickUpStones::ConsumeStones()
{
	//If this array is empty there are no more stones in the level
	if (CollectedStones.IsEmpty())
	{
		//We cancel the ability
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		return;
	}
	
	//Range based for loop that goes through the array of CollectedStones
	for (AWarriorStoneBase* CollectedStone : CollectedStones)
	{
		//Check if collected stone is valid
		if (CollectedStone)
		{
			//Call the consume function from our stone class
			CollectedStone->Consume(GetWarriorAbilitySystemComponentFromActorInfo(), GetAbilityLevel());	
		}
	}
}
