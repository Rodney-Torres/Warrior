// Rodney Torres All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "WarriorGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FWarriorGameLevelSet
{
	GENERATED_BODY()
	
	//tag were going to use to retrieve our level
	UPROPERTY(EditDefaultsOnly, meta = (Categories = "GameData.Level"))
	FGameplayTag LevelTag;
	
	//soft reference variable for the level
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> Level;
	
	//helper function to check if valid and not null
	bool IsValid() const
	{
		return LevelTag.IsValid() && !Level.IsNull();
	}
	
	
};

/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
	//2 publics here im not sure why. probably a mistake from lectures
public:
	//Function involved in our loading screen
	virtual void Init() override;
	
protected:
	//delegates for our loading screen
	virtual void OnPreLoadMap (const FString& MapName);
	virtual void OnDestinationWorldLoaded(UWorld* LoadedWorld);
	
	//Array of our struct which we will need to assign valid levels to in our editor later
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FWarriorGameLevelSet> GameLevelSets;
	
public:
	//Soft getter for level through tag
	UFUNCTION(BlueprintPure, meta = (GameplayTagFilter = "GameData.Level"))
	TSoftObjectPtr<UWorld> GetGameLevelByTag(FGameplayTag InTag) const;
};
