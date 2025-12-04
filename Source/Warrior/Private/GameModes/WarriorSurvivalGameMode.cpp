// Rodney Torres All Rights Reserved


#include "GameModes/WarriorSurvivalGameMode.h"

void AWarriorSurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	//In order for us to continue we must have a valid DT in BP. We trigger a assertion if we don't.
	checkf(EnemyWaveSpawnerDataTable, TEXT("Forgot to assign a valid DT in survival game mode BP"));
	
	//Setting our state on startup
	SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::WaitSpawnNewWave);
	
	//This function will return an array filled with all the available row names. We only want to know the total amount of elements in the array
	//We then store this inside our member variable. Now in CPP we know the total amount of waves
	TotalWavesToSpawn = EnemyWaveSpawnerDataTable->GetRowNames().Num();
}

void AWarriorSurvivalGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//Here we will handle advancing into the next wave
	if (CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::WaitSpawnNewWave)
	{
		//If yes we start elapsed timer, check if elapsed timer passes a threshold, if yes we reset elapsed timer and change enum
		
		TimePassedSinceStart += DeltaTime;
		
		if (TimePassedSinceStart >= SpawnNewWaveWaitTime)
		{
			TimePassedSinceStart = 0.f;
			
			SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::SpawningNewWave);
		}
	}

	//Here we will handle spawning a new wave
	if (CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::SpawningNewWave)
	{
		//if yes start elapsed timer and check if timer has passed a threshold. If yes spawn enemies, reset elapsed time, and change enum
		TimePassedSinceStart += DeltaTime;
		
		if (TimePassedSinceStart >= SpawnEnemiesDelayTime)
		{
			//TODO: Handle spawn new enemies
			
			TimePassedSinceStart = 0.f;
			
			SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::InProgress);
		}
	}
	//Here we will handle what to do when the wave is completed
	if (CurrentSurvivalGameModeState == EWarriorSurvivalGameModeState::WaveCompleted)
	{
		//Start elapsed timer, check if timer passes threshold, if yes reset timer, increment WaveCount, and we check if we finished all waves
		//We do this with a bool function that checks if our current wave count is > total wave count. If it is we win.
		//If it isn't then we restart this whole handling of the waves by going back to the enum WaitSpawnNewWave and restarting this cpp process
		TimePassedSinceStart += DeltaTime;
		
		if (TimePassedSinceStart >= WaveCompletedWaitTime)
		{
			TimePassedSinceStart = 0.f;
			
			CurrentWaveCount++;

			if (HasFinishedAllWaves())
			{
				SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::AllWavesDone);
			}
			else
			{
				SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::WaitSpawnNewWave);
			}
		}
	}
}

//This function will be called in multiple places in the class
void AWarriorSurvivalGameMode::SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState InState)
{
	//Set our current state to the InState
	CurrentSurvivalGameModeState = InState;
	
	//Broadcast our delegate as well
	OnSurvivalGameModeStateChanged.Broadcast(CurrentSurvivalGameModeState);
}

//If bool is true that means we finished all waves 
bool AWarriorSurvivalGameMode::HasFinishedAllWaves() const
{
	return CurrentWaveCount > TotalWavesToSpawn;
}
