// Rodney Torres All Rights Reserved


#include "GameModes/WarriorBaseGameMode.h"

AWarriorBaseGameMode::AWarriorBaseGameMode()
{
	//2 things we need to set to enable tick here to fix our survival game mode child class
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}
