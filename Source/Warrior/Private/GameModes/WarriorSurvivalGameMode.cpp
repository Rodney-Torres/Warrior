// Rodney Torres All Rights Reserved


#include "GameModes/WarriorSurvivalGameMode.h"
#include "Engine/AssetManager.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"

#include "WarriorDebugHelper.h"

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
	
	//Calling this to preload the first waves enemies
	PreLoadNextWaveEnemies();
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
			//We will make use of the counter later
			CurrentSpawnedEnemiesCounter += TrySpawnWaveEnemies();
			
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
		//We also preload the next wave enemies in that else case
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
				//This should take care of loading in the next wave
				PreLoadNextWaveEnemies();
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

//I dont understand this too well
void AWarriorSurvivalGameMode::PreLoadNextWaveEnemies()
{
	//The SoftClassPointers are inside our data table and it consists of different rows. In order to access this array since we have many
	//different rows we will need to know what row were trying to access here and to do that we will make a new helper function to do that
	
	//Check if all waves are finished is true to return early
	if (HasFinishedAllWaves())
	{
		return;
	}
	
	//We should empty our TMap here, this ensures we always have an empty TMap to begin with
	PreLoadedEnemyClassMap.Empty();
	
	//Element type is what is inside our array. We loop through the array and EnemyWaveSpawnerDefinitions.
	for (const FWarriorEnemyWaveSpawnerInfo& SpawnerInfo : GetCurrentWaveSpawnerTableRow()->EnemyWaveSpawnerDefinitions)
	{
		if (SpawnerInfo.SoftEnemyClassToSpawn.IsNull()) continue;

		UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SpawnerInfo.SoftEnemyClassToSpawn.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda(
			[SpawnerInfo, this]()
			{
				//If true our enemy class is loaded. We need to store this in a member variable so we can use it for spawning later
				if (UClass* LoadedEnemyClass = SpawnerInfo.SoftEnemyClassToSpawn.Get())
				{
					PreLoadedEnemyClassMap.Emplace(SpawnerInfo.SoftEnemyClassToSpawn, LoadedEnemyClass);
				}
			}
			)
		);
	}
}

FWarriorEnemyWaveSpawnerTableRow* AWarriorSurvivalGameMode::GetCurrentWaveSpawnerTableRow() const
{
	//Create the RowName using the correct naming convention + Find the row and set it to FoundRow + check if FoundRow is valid + return FoundRow
	const FName RowName = FName(TEXT("Wave") + FString::FromInt(CurrentWaveCount));
	
	FWarriorEnemyWaveSpawnerTableRow* FoundRow = EnemyWaveSpawnerDataTable->FindRow<FWarriorEnemyWaveSpawnerTableRow>(RowName, FString());
	
	checkf(FoundRow, TEXT("Could not find a valid row under the name %s in the data table"), *RowName.ToString());
	
	return FoundRow;
}

int32 AWarriorSurvivalGameMode::TrySpawnWaveEnemies()
{
	//Retrieve target points in the level
	
	//If array is empty then we're finding target points and filling an array with them
	if (TargetPointsArray.IsEmpty())
	{
		UGameplayStatics::GetAllActorsOfClass(this, ATargetPoint::StaticClass(),TargetPointsArray );
	}

	//If after this it is still empty it means there are no available target points in the level so we will trigger an assertion
	checkf(!TargetPointsArray.IsEmpty(), TEXT("No valid target point found in level %s for spawning enemies"), *GetWorld()->GetName());
	
	uint32 EnemiesSpawnedThisTime = 0;
	
	//Spawn params for spawning the enemies, all we do is change the collision handling override
	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//Retrieving current table row for spawning here and looping through it.
	for (const FWarriorEnemyWaveSpawnerInfo& SpawnerInfo : GetCurrentWaveSpawnerTableRow()->EnemyWaveSpawnerDefinitions)
	{
		//We check if its empty
		if (SpawnerInfo.SoftEnemyClassToSpawn.IsNull()) continue;
		
		//here we figure out how many enemies we will be spawning
		const int32 NumToSpawn = FMath::RandRange(SpawnerInfo.MinPerSpawnCount, SpawnerInfo.MaxPerSpawnCount);
		
		//here we retrieve the enemy class to spawn by accessing the TMap we created and store it in a local variable
		//Using FingChecked triggers a assertion if there is no valid value with the key
		UClass* LoadedEnemyClass = PreLoadedEnemyClassMap.FindChecked(SpawnerInfo.SoftEnemyClassToSpawn);
		
		//First time I remember using a loop like this 
		for (int32 i = 0; i < NumToSpawn; ++i)
		{
			//getting ahold of this index
			const int32 RandomTargetPointIndex = FMath::RandRange(0,TargetPointsArray.Num() -1);
			//putting it inside an array
			const FVector SpawnOrigin = TargetPointsArray[RandomTargetPointIndex]->GetActorLocation();
			//Get ahold of a valid spawn rotation
			const FRotator SpawnRotation = TargetPointsArray[RandomTargetPointIndex]->GetActorForwardVector().ToOrientationRotator();
			
			//Local variable for the out param of random location
			FVector RandomLocation;
			//With the SpawnOrigin we need to randomize the final spawn location
			UNavigationSystemV1::K2_GetRandomLocationInNavigableRadius(this, SpawnOrigin, RandomLocation, 400.f);
			
			//Give this random location some Z offset and this is our final spawn location
			RandomLocation += FVector(0.f, 0.f, 150.f);
			
			//Spawn the enemies, spawn params is created outside the for loop. Store it in a local variable
			AWarriorEnemyCharacter* SpawnedEnemy = GetWorld()->SpawnActor<AWarriorEnemyCharacter>(LoadedEnemyClass, RandomLocation, SpawnRotation, SpawnParam);
			
			//Our gamemode needs to know when an enemy has died and there is a perfect delegate inside the actor class that can do just that
			//If spawned enemy is valid
			if (SpawnedEnemy)
			{
				//Our gamemode needs to know when an enemy has died and there is a perfect delegate inside the actor class that can do just that
				//OnDestroyed event gets called when the actor has been destroyed
				//Now our gamemode knows when an enemy has been destroyed
				SpawnedEnemy->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnEnemyDestroyed);
				//we increase these 2 counter
				EnemiesSpawnedThisTime++;
				TotalSpawnedEnemiesThisWaveCounter++;
			}
			
			//if false we... if true we keep looping and keep spawning
			if (!ShouldKeepSpawnEnemies())
			{
				return EnemiesSpawnedThisTime;
			}
		}
	}
	
	return EnemiesSpawnedThisTime;
	
}

//helper function for checking if we have spawned enough
bool AWarriorSurvivalGameMode::ShouldKeepSpawnEnemies() const
{
	//If this condition returns false then we should no longer be spawning enemies
	return TotalSpawnedEnemiesThisWaveCounter < GetCurrentWaveSpawnerTableRow()->TotalEnemyToSpawnThisWave;
}

void AWarriorSurvivalGameMode::OnEnemyDestroyed(AActor* DestroyedActor)
{
	//decrease counter
	CurrentSpawnedEnemiesCounter--;
	
	// - for testing - Debug::Print(FString::Printf(TEXT("CurrentSpawnedEnemiesCounter:%i, TotalSpawnedEnemiesThisWaveCounter%i"), CurrentSpawnedEnemiesCounter, TotalSpawnedEnemiesThisWaveCounter));
	
	//if this is true that means we need to spawn more enemies this wave
	if (ShouldKeepSpawnEnemies())
	{
		CurrentSpawnedEnemiesCounter += TrySpawnWaveEnemies();
	}
	//If the above is false and if the counter is 0 then there is no more enemies in the level so....
	else if (CurrentSpawnedEnemiesCounter == 0)
	{
		//We should set the counters to 0 and set the enum correctly
		TotalSpawnedEnemiesThisWaveCounter = 0;
		CurrentSpawnedEnemiesCounter = 0;
		
		SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState::WaveCompleted);
	}
}

void AWarriorSurvivalGameMode::RegisterSpawnedEnemies(const TArray<AWarriorEnemyCharacter*>& InEnemiesToRegister)
{
	//loop through array for SpawnedEnemy
	for (AWarriorEnemyCharacter* SpawnedEnemy : InEnemiesToRegister)
	{
		//if it is valid
		if (SpawnedEnemy)
		{
			//add to counter
			CurrentSpawnedEnemiesCounter++;
			//bind the spawned enemy to this delegate that handles enemy death in gamemode
			SpawnedEnemy->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnEnemyDestroyed);
		}
	}
}
