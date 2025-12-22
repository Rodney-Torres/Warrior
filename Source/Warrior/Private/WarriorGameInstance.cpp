// Rodney Torres All Rights Reserved


#include "WarriorGameInstance.h"
#include "MoviePlayer.h"

//function involved in our loading screen
void UWarriorGameInstance::Init()
{
	Super::Init();
	
	//2 delegates for our opening and closing our loading screen
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ThisClass::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnDestinationWorldLoaded);
}

//opens our loading screen
void UWarriorGameInstance::OnPreLoadMap(const FString& MapName)
{
	//inside this struct we can configure our loading screen
	// 1 - completes when loading completes. 2 - minimum display time 3 - content of our loading screen/things we want to display
	FLoadingScreenAttributes LoadingScreenAttributes;
	LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = true;
	LoadingScreenAttributes.MinimumLoadingScreenDisplayTime = 2.f;
	//proper way to construct this is creating a separate module and using cpp code to create the widget. But since writing slate code is out of scope for this course we won't do it.
	LoadingScreenAttributes.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
	
	//calling this function to set up our loading screen. Setting it up here ensures our loading screen runs on a different thread
	//No matter what happens on our game thread it wont cause a hitch on our loading screen
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
}

//closes our loading screen
void UWarriorGameInstance::OnDestinationWorldLoaded(UWorld* LoadedWorld)
{
	GetMoviePlayer()->StopMovie();
}

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
