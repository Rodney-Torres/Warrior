// Rodney Torres All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WarriorBaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorBaseGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	//Created a constructor, this was for adding tick to the parent class of our survival game mode. Since it wasnt working
	AWarriorBaseGameMode();
};
