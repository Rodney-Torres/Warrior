// Rodney Torres All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameModes/WarriorBaseGameMode.h"
#include "WarriorSurvivalGameMode.generated.h"

class AWarriorEnemyCharacter;
//Enum for state of the game mode
UENUM(BlueprintType)
enum class EWarriorSurvivalGameModeState : uint8
{
	WaitSpawnNewWave,
	SpawningNewWave,
	InProgress,
	WaveCompleted,
	AllWavesDone,
	PlayerDied
};

//Now based off this struct we can start to define our data table based on this struct
USTRUCT(BlueprintType)
struct FWarriorEnemyWaveSpawnerInfo
{
	GENERATED_BODY()
	
	//Enemy we will be spawning
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AWarriorEnemyCharacter> SoftEnemyClassToSpawn;
	
	UPROPERTY(EditAnywhere)
	int32 MinPerSpawnCount = 1;
	
	UPROPERTY(EditAnywhere)
	int32 MaxPerSpawnCount = 3;
};

//This inherits from FTableRowBase and this is how we can create a data table from this struct and is the complete table row created
USTRUCT(BlueprintType)
struct FWarriorEnemyWaveSpawnerTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TArray<FWarriorEnemyWaveSpawnerInfo> EnemyWaveSpawnerDefinitions;
	
	UPROPERTY(EditAnywhere)
	int32 TotalEnemyToSpawnThisWave = 1;
};


//Delegate broadcasting our enum, this is the delegate we will use to notify out BP to do the things needed.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalGameModeStateChangedDelegate, EWarriorSurvivalGameModeState, CurrentState);

/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorSurvivalGameMode : public AWarriorBaseGameMode
{
	GENERATED_BODY()
	
protected:
	//Used to kick off the game loop
	virtual void BeginPlay() override;
	//Used to handle wave progression
	virtual void Tick(float DeltaTime) override;
	
private:
	//Setter for setting our current survival game mode state and this is not something we need outside of this class its for easy access in this one
	void SetCurrentSurvivalGameModeState(EWarriorSurvivalGameModeState InState);
	
	//Variable used to keep track of the current state
	UPROPERTY()
	EWarriorSurvivalGameModeState CurrentSurvivalGameModeState;
	
	//Variable used to keep track of the gamemode state changing. I think.
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSurvivalGameModeStateChangedDelegate OnSurvivalGameModeStateChanged;
	
	//Since were creating a BPReadOnly in the private section we need to add another meta specifier
	//Later we need to create this data table in the editor and assign it in BP
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	UDataTable* EnemyWaveSpawnerDataTable;
};
