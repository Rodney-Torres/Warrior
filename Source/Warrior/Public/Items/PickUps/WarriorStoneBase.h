// Rodney Torres All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Items/PickUps/WarriorPickUpBase.h"
#include "WarriorStoneBase.generated.h"

class UWarriorAbilitySystemComponent;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorStoneBase : public AWarriorPickUpBase
{
	GENERATED_BODY()
	
public:
	//This is the function we will call from our gameplay ability later
	void Consume(UWarriorAbilitySystemComponent* AbilitySystemComponent, int32 ApplyLevel);
	
protected:
	virtual void OnPickUpCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
	//We need to handle spawning the VFX and play a sound FX for consuming and destroy our stone. We do this in BP
	//This is the function we will override later inside of our BP class. We call this function inside of the Consume function after applying the GE
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStoneConsumed"))
	void BP_OnStoneConsumed();
	
	//Variable to specify which gameplay effect to apply when consumed
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> StoneGameplayEffectClass;
};
