// Rodney Torres All Rights Reserved


#include "WarriorFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Interfaces/PawnCombatInterface.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "WarriorGameplayTags.h"
#include "WarriorTypes/WarriorCountDownAction.h"
#include "WarriorGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGame/WarriorSaveGame.h"

#include "WarriorDebugHelper.h"

UWarriorAbilitySystemComponent* UWarriorFunctionLibrary::NativeGetWarriorASCFromActor(AActor* InActor)
{
    check(InActor);

   return CastChecked<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor));
}

void UWarriorFunctionLibrary::AddGameplayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd)
{
    UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);

    if (!ASC->HasMatchingGameplayTag(TagToAdd))
    {
        ASC->AddLooseGameplayTag(TagToAdd);
    }
}

void UWarriorFunctionLibrary::RemoveGameplayTagFromActorIfFound(AActor* InActor, FGameplayTag TagToRemove)
{
    UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);

    if (ASC->HasMatchingGameplayTag(TagToRemove))
    {
        ASC->RemoveLooseGameplayTag(TagToRemove);
    }
}

bool UWarriorFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck)
{
    UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);

    return ASC->HasMatchingGameplayTag(TagToCheck);
}

void UWarriorFunctionLibrary::BP_DoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck, EWarriorConfirmType& OutConfirmType)
{
    OutConfirmType = NativeDoesActorHaveTag(InActor, TagToCheck) ? EWarriorConfirmType::Yes : EWarriorConfirmType:: No;
}

UPawnCombatComponent* UWarriorFunctionLibrary::NativeGetPawnCombatComponentFromActor(AActor* InActor)
{
    check(InActor);

    if (IPawnCombatInterface* PawnCombatInterface = Cast<IPawnCombatInterface>(InActor))
    {
        return PawnCombatInterface->GetPawnCombatComponent();
    }

    return nullptr;
}

UPawnCombatComponent* UWarriorFunctionLibrary::BP_GetPawnCombatComponentFromActor(AActor* InActor, EWarriorValidType& OutValidType)
{
    UPawnCombatComponent* CombatComponent = NativeGetPawnCombatComponentFromActor(InActor);

    OutValidType = CombatComponent ? EWarriorValidType::Valid : EWarriorValidType::Invalid;

    return CombatComponent;
}

bool UWarriorFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
	check(QueryPawn && TargetPawn);

	IGenericTeamAgentInterface* QueryTeamAgent = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController());
    IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController());

    if (QueryTeamAgent && TargetTeamAgent)
    {
        return  QueryTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
    }

    return false;
}

float UWarriorFunctionLibrary::GetScalableFloatValueAtLevel(const FScalableFloat& InScalableFloat, float InLevel)
{
	return InScalableFloat.GetValueAtLevel(InLevel);
}

FGameplayTag UWarriorFunctionLibrary::ComputeHitReactDirectionTag(AActor* InAttacker, AActor* InVictim, float& OutAngleDifference)
{
	check(InAttacker && InVictim);

	const FVector VictimForward = InVictim->GetActorForwardVector();
	const FVector VictimToAttackerNormalized = (InAttacker->GetActorLocation() - InVictim->GetActorLocation()).GetSafeNormal();

    const float DotResult = FVector::DotProduct(VictimForward, VictimToAttackerNormalized);
    OutAngleDifference = UKismetMathLibrary::DegAcos(DotResult);

    const FVector CrossResult = FVector::CrossProduct(VictimForward, VictimToAttackerNormalized);

    if (CrossResult.Z < 0.f)
    {
        OutAngleDifference *= -1.f;
    }

    if (OutAngleDifference >= -45.f && OutAngleDifference <= 45.F)
    {
        return WarriorGameplayTags::Shared_Status_HitReact_Front;
    }
    else if (OutAngleDifference < -45.f && OutAngleDifference >= -135.f)
    {
		return WarriorGameplayTags::Shared_Status_HitReact_Left;
    }
    else if (OutAngleDifference < -135.f || OutAngleDifference > 135.f)
    {
        return WarriorGameplayTags::Shared_Status_HitReact_Back;
    }
    else if (OutAngleDifference > 45.f && OutAngleDifference <= 135.f)
    {
        return WarriorGameplayTags::Shared_Status_HitReact_Right;
    }

    return WarriorGameplayTags::Shared_Status_HitReact_Front;
}

bool UWarriorFunctionLibrary::IsValidBlock(AActor* InAttacker, AActor* InDefender)
{
	check(InAttacker && InDefender);

    const float DotResult = FVector::DotProduct(InAttacker->GetActorForwardVector(), InDefender->GetActorForwardVector());

	/* const FString DebugString = FString::Printf(TEXT("Dot Result: %f %s"), DotResult, DotResult < -0.1f ? TEXT("Valid Block") : TEXT("Invalid Block"));

    Debug::Print(DebugString, DotResult < -0.1f ? FColor::Green : FColor::Red); */

    return DotResult < -0.1f;
}

bool UWarriorFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor(AActor* InInstigator, AActor* InTargetActor, const FGameplayEffectSpecHandle& InSpecHandle)
{
    UWarriorAbilitySystemComponent* SourceASC = NativeGetWarriorASCFromActor(InInstigator);
    UWarriorAbilitySystemComponent* TargetASC = NativeGetWarriorASCFromActor(InTargetActor);

   FActiveGameplayEffectHandle ActiveGameplayEffectHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*InSpecHandle.Data, TargetASC);

   return ActiveGameplayEffectHandle.WasSuccessfullyApplied();
}

void UWarriorFunctionLibrary::CountDown(const UObject* WorldContextObject, float TotalTime, float UpdateInterval, float& OutRemainingTime, EWarriorCountDownActionInput CountDownInput, UPARAM(DisplayName = "Output") EWarriorCountDownActionOutput& CountDownOutput, FLatentActionInfo LatentInfo)
{
    UWorld* World = nullptr;
    
    if (GEngine)
    {
        //Inside EGetWorldErrorMode we can specify what happens when the world we get is not valid. We can trigger a assertion or log and return null
        //We store the return in our local variable World
        World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    }
    
    if (!World)
    {
        return;
    }
    
    //Once our world is valid we can retrive our latent action manager. We have to store this inside a reference variable since it returns a reference variable.
    FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
    
    //For this function the first thing we should do is specify the action type. To do that we need to include the header file for our action
    //Inside of our () we need to find 2 things, the first is our InActionObject from our function input which is LatentInfo
    //The 2nd is a UUID which is a unique id that we can use to retrieve our existing latent action, we can get this from our LatentInfo as well.
    //Then we will store the return inside another local variable
    FWarriorCountDownAction* FoundAction = LatentActionManager.FindExistingAction<FWarriorCountDownAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
    
    //Next we should check which input pin was triggered inside our BP. To do that we need to check our function input CountDownInput
    //If we make it inside this if check we hit the start input pin. So its important to check if this found action is not valid
   if (CountDownInput == EWarriorCountDownActionInput::Start)
   {
       //When this is not valid we will try to create a new one through our latentactionmanager and call the function add new action
       if (!FoundAction)
       {
           //This takes a lot of params. The first 2 we can get from our latentinfo struct which is the callback target and the uuid
           //For the other ones we need to construct our countdown action which we can use the new keyword for that.
           //This action is gonna take in a lot of params we created for it
           LatentActionManager.AddNewAction(
               LatentInfo.CallbackTarget, 
               LatentInfo.UUID,
               //This is how we can construct our countdown action inside of our static function
               //The new keyword will not create a memory leak because it will be managed by the LatentActionManager automatically
               //This is the start input pin taken care of
               new FWarriorCountDownAction(TotalTime, UpdateInterval, OutRemainingTime, CountDownOutput, LatentInfo)
               );
       }
   }
    //This means we need to cancel this count down action
    if (CountDownInput == EWarriorCountDownActionInput::Cancel)
    {
        //If FoundAction is valid in this we need to cancel it. To do that were going to create a function inside our action class called CancelAction
        if (FoundAction)
        {
            FoundAction->CancelAction();
        }
    }
    
}

//getting game instance helper function for widget buttons to retrieve and load this game instance when pressed
UWarriorGameInstance* UWarriorFunctionLibrary::GetWarriorGameInstance(const UObject* WorldContextObject)
{
    if (GEngine)
    {
        if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
        {
            return World->GetGameInstance<UWarriorGameInstance>();
        }
    }
    
    return nullptr;
}

void UWarriorFunctionLibrary::ToggleInputMode(const UObject* WorldContextObject, EWarriorInputMode InInputMode)
{
    //creating local variable that is empty for the PlayerController
    APlayerController* PlayerController = nullptr;

    if (GEngine)
    {
        //use g engine to retrieve the world from the world context object
        if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
        {
            //use world to retrieve the player controller and set it as the PlayerController local variable
            PlayerController = World->GetFirstPlayerController();
        }
    }

    //return early if not valid
    if (!PlayerController)
    {
        return;
    }
    
    //To satisfy our function SetInputMode param we will create a FInputModeGameOnly variable here
    FInputModeGameOnly GameOnlyMode;
    FInputModeUIOnly UIOnlyMode;
    
    //now after these checks we can start to toggle our input modes using a switch case
    switch (InInputMode)
    {
    case EWarriorInputMode::GameOnly:
        
        //Set input mode and set mouse cursor bool
        PlayerController->SetInputMode(GameOnlyMode);
        PlayerController->bShowMouseCursor = false;
        
        break;
    case EWarriorInputMode::UIOnly:
        
        //Set input mode and set mouse cursor bool
        PlayerController->SetInputMode(UIOnlyMode);
        PlayerController->bShowMouseCursor = true;
        
        break;
    default:
        break;
    }
}

//This is how we can save our game difficulty. We will create a save game object first. Then we will cast it to our WarriorSaveGame class
//Then we will set the variable inside our save game class to the input param we have here and finally we will call the save game to slot function 
//to save it synchronously
void UWarriorFunctionLibrary::SaveCurrentGameDifficulty(EWarriorGameDifficulty InDifficultyToSave)
{
    USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(UWarriorSaveGame::StaticClass());

    if (UWarriorSaveGame* WarriorSaveGameObject = Cast<UWarriorSaveGame>(SaveGameObject))
    {
        WarriorSaveGameObject->SavedCurrentGameDifficulty = InDifficultyToSave;
        
        const bool bWasSaved = UGameplayStatics::SaveGameToSlot(WarriorSaveGameObject , WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);
        
        Debug::Print(bWasSaved? TEXT("Difficulty Saved") : TEXT("Difficulty Not Saved"));
    }
}

//this is how we load our game difficulty. We will first check if the save game exists in the specified slot. If it does we will load it from the slot. 
//Then we will cast it to our WarriorSaveGame class. 
//If the cast is successful we will set the output param to the saved difficulty variable inside our save game class.
//Finally, we will return true and if the savegame does not exist we return false.
bool UWarriorFunctionLibrary::TryLoadSavedGameDifficulty(EWarriorGameDifficulty& OutSavedDifficulty)
{
   if (UGameplayStatics::DoesSaveGameExist(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0))
   {
       USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);
       
       if (UWarriorSaveGame* WarriorSaveGameObject = Cast<UWarriorSaveGame>(SaveGameObject))
       {
           OutSavedDifficulty = WarriorSaveGameObject->SavedCurrentGameDifficulty;
           
           Debug::Print(TEXT("Loading Successful"), FColor::Green);
           
           return true;
       }
   }
    
    return false;
}



