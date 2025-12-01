// Rodney Torres All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "HeroGameplayAbility_PickUpStones.generated.h"

class AWarriorStoneBase;
/**
 * 
 */
UCLASS()
class WARRIOR_API UHeroGameplayAbility_PickUpStones : public UWarriorHeroGameplayAbility
{
	GENERATED_BODY()
	
protected:
	//~ Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End UGameplayAbility Interface
	
	//Function is called every frame later and needs no inputs
	UFUNCTION(BlueprintCallable)
	void CollectStones();
	
	//Function called after we recieve the gameplay event
	UFUNCTION(BlueprintCallable)
	void ConsumeStones();
	
private:
	//float variable to extend the trace downwards
	UPROPERTY(EditDefaultsOnly)
	float BoxTraceDistance = 50.f;
	
	//Variable for the box trace size, used for the half size input we divide by 2
	UPROPERTY(EditDefaultsOnly)
	FVector TraceBoxSize = FVector(100.f);
	
	//Object types were looking for in our box trace
	UPROPERTY(EditDefaultsOnly)
	TArray<TEnumAsByte<	EObjectTypeQuery> > StoneTraceChannel;
	
	//Debug bool to toggle drawing the box trace
	UPROPERTY(EditDefaultsOnly)
	bool bDrawDebugShape = false;
	
	//Array to store the found stones
	UPROPERTY()
	TArray<AWarriorStoneBase*> CollectedStones;
};
