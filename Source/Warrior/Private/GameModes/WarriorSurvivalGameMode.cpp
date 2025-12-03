// Rodney Torres All Rights Reserved


#include "GameModes/WarriorSurvivalGameMode.h"

void AWarriorSurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AWarriorSurvivalGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//This function will be called in multiple places in the class
void AWarriorSurvivalGameMode::SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState InState)
{
	//Set our current state to the InState
	CurrentSurvivalGameModeState = InState;
	
	//Broadcast our delegate as well
	OnSurvivalGameModeStateChanged.Broadcast(CurrentSurvivalGameModeState);
}
