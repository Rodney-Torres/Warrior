// Rodney Torres All Rights Reserved


#include "Items/PickUps/WarriorStoneBase.h"
#include "Characters/WarriorHeroCharacter.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "WarriorGameplayTags.h"

void AWarriorStoneBase::Consume(UWarriorAbilitySystemComponent* AbilitySystemComponent, int32 ApplyLevel)
{
	//Before we apply the GE we need to make sure the StoneGameplayEffectClass is valid
	check(StoneGameplayEffectClass);
	
	//Then we retrieve the class default object from this effect class through a template function and store it in a local variable
	UGameplayEffect* EffectCDO = StoneGameplayEffectClass->GetDefaultObject<UGameplayEffect>();
	
	//So we apply the effect to the hero character here
	AbilitySystemComponent->ApplyGameplayEffectToSelf(
		EffectCDO, //GameplayEffect we want to apply which is decided beforehand
		ApplyLevel, //Function input
		AbilitySystemComponent->MakeEffectContext() // using ASC to make this variable (I dont know what EffectContext is)
	);
	
	//Call the BP event to handle VFX and SFX
	BP_OnStoneConsumed();
}

void AWarriorStoneBase::OnPickUpCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                                            AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                            const FHitResult& SweepResult)
{
	// Rider added - Super::OnPickUpCollisionSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	//Our stones should only respond to our hero character because of this. Then we store it inside a local variable
	if (AWarriorHeroCharacter* OverlappedHeroCharacter = Cast<AWarriorHeroCharacter>(OtherActor))
	{
		//From this we get the ASC of the hero character and use the helper function to try to activate the ability by tag
		//This is how we can activate the ability from our stone class
		OverlappedHeroCharacter->GetWarriorAbilitySystemComponent()->TryActivateAbilityByTag(WarriorGameplayTags::Player_Ability_PickUp_Stones);
	}
}
