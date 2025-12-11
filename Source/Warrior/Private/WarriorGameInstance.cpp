// Rodney Torres All Rights Reserved


#include "WarriorGameInstance.h"

TSoftObjectPtr<UWorld> UWarriorGameInstance::GetGameLevelByTag(FGameplayTag InTag) const
{
	//loop through our array
	for (const FWarriorGameLevelSet& GameLevelSet : GameLevelSets)
	{
		//check if not valid
		if (!GameLevelSet.IsValid()) continue;

		//check if tag is same and return the level
		if (GameLevelSet.LevelTag == InTag)
		{
			return GameLevelSet.Level;
		}
	}
	
	//return empty soft ptr
	return TSoftObjectPtr<UWorld>();
}
