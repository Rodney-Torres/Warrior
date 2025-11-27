// Rodney Torres All Rights Reserved


#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"

void UWarriorGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (AbilityActivationPolicy == EWarriorAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo && !Spec.IsActive())
		{
			ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
		}
	}
}

void UWarriorGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AbilityActivationPolicy == EWarriorAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo)
		{
			ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
		}
	}
}

UPawnCombatComponent* UWarriorGameplayAbility::GetPawnCombatComponentFromActorInfo() const
{
	return GetAvatarActorFromActorInfo()->FindComponentByClass<UPawnCombatComponent>();
}

UWarriorAbilitySystemComponent* UWarriorGameplayAbility::GetWarriorAbilitySystemComponentFromActorInfo() const
{
	return Cast<UWarriorAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent);
}

FActiveGameplayEffectHandle UWarriorGameplayAbility::NativeApplyEffectSpecHandleToTarget(AActor* TargetActor, const FGameplayEffectSpecHandle& InSpecHandle)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	check(TargetASC && InSpecHandle.IsValid());

	return GetWarriorAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
		*InSpecHandle.Data,
		TargetASC
	);
}

FActiveGameplayEffectHandle UWarriorGameplayAbility::BP_ApplyEffectSpecHandleToTarget(AActor* TargetActor, const FGameplayEffectSpecHandle& InSpecHandle, EWarriorSuccessType& OutSuccessType)
{
	FActiveGameplayEffectHandle ActiveGameplayEffectHandle = NativeApplyEffectSpecHandleToTarget(TargetActor, InSpecHandle);

	OutSuccessType = ActiveGameplayEffectHandle.WasSuccessfullyApplied() ? EWarriorSuccessType::Successful : EWarriorSuccessType::Failed;

	return ActiveGameplayEffectHandle;
}

void UWarriorGameplayAbility::ApplyGameplayEffectSpecHandleToHitResults(const FGameplayEffectSpecHandle& InSpecHandle, const TArray<FHitResult>& InHitResults)
{
	//return early if the array is empty
	if (InHitResults.IsEmpty())
	{
		return;
	}
	
	//Casting so we can get ahold of the QueryPawn for the hostile check. We use CastChecked so we dont need to check the owning pawn again.
	APawn* OwningPawn = CastChecked<APawn>(GetAvatarActorFromActorInfo());
	
	//loop through the array and check if the actor were hitting is a pawn, if so we store it as a local variable and then...
	for (const FHitResult& Hit : InHitResults)
	{
		if (APawn* HitPawn = Cast<APawn>(Hit.GetActor()))
		{
			//check if the HitPawn is hostile using our own helper function from the function library
			if (UWarriorFunctionLibrary::IsTargetPawnHostile(OwningPawn, HitPawn))
			{
				//If pawn is hostile then we apply the effect spec handle to the target
				//To do that we have a function for that inside our WarriorGameplayAbility class
				//We store the return result inside of a local variable
				FActiveGameplayEffectHandle ActiveGameplayEffectHandle = NativeApplyEffectSpecHandleToTarget(HitPawn, InSpecHandle);
				
				//From this local variable we can know if this gameplay effect handle was successfully applied
				if (ActiveGameplayEffectHandle.WasSuccessfullyApplied())
				{
					//Created event data for the hit react event
					FGameplayEventData Data;
					Data.Instigator = OwningPawn;
					Data.Target = HitPawn;
					
					//Notify hit pawn to do a hit react which we will do by sending a gameplay event to the actor
					//For this we need to create a event data for it which we do above this function
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
					HitPawn,
					WarriorGameplayTags::Shared_Event_HitReact,
					Data
					);
				}
			}
		}
	}
}
