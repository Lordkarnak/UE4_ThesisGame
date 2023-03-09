// Fill out your copyright notice in the Description page of Project Settings.


#include "ThesisGameInstance.h"
#include "ThesisGameGameState.h"
#include "ThesisSaveGame.h"
#include "Characters/PlayableCharacter.h"
#include "Characters/BotCharacter.h"
#include "Quests/Quest.h"
#include "Pickups/SaveTerminal.h"
#include "Engine.h"

void UThesisGameInstance::Init()
{
	PendingSave = false;
	PendingLoad = false;
	FirstPlayerStartName = FName(TEXT("DefaultStart"));
	CheckpointPlayerStartName = FName(TEXT("CheckpointStart"));
	bIsLoadedGame = false;
}

bool UThesisGameInstance::SaveGame(ASaveTerminal* Caller, const FString& SlotName, const int32 Index, bool bUseAsync)
{
	if (!SlotName.IsEmpty() && Index > -1)
	{
		GameData = Cast<UThesisSaveGame>(UGameplayStatics::CreateSaveGameObject(UThesisSaveGame::StaticClass()));
		// Proceed to save game data
		UWorld* CurrentWorld = GetWorld();
		if (CurrentWorld != nullptr)
		{
			ACharacter* Player = UGameplayStatics::GetPlayerCharacter(CurrentWorld, 0);
			AThesisGameGameState* CurrentState = CurrentWorld->GetGameState<AThesisGameGameState>();
			
			// Basic Data...
			GameData->LastLevel = CurrentWorld->GetFName();
			if (Caller)
			{
				GameData->LastCheckpoint = Caller->LinkedCheckpoint;
			}
			/*GameData->PlayerLocation = Player->GetActorLocation();
			GameData->PlayerLocation.Z += Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();*/

			// Quests...
			AQuest* ActiveQuest = CurrentState->GetActiveQuest();
			if (ActiveQuest) { GameData->ActiveQuestData = FQuestData(ActiveQuest); }
			
			TArray<AQuest*> CurrentQuests = CurrentState->GetCurrentQuests();
			if (CurrentQuests.Num() > 0)
			{
				for (int32 i = 0; i < CurrentQuests.Num(); i++)
				{
					if (CurrentQuests[i]) { GameData->CurrentQuestsData.Add(FQuestData(CurrentQuests[i])); }
				}
			}
			
			TArray<AQuest*> CompletedQuests = CurrentState->GetCompletedQuests();
			if (CompletedQuests.Num() > 0)
			{
				for (int32 i = 0; i < CompletedQuests.Num(); i++)
				{
					if (CompletedQuests[i]) { GameData->CompletedQuestsData.Add(FQuestData(CompletedQuests[i])); }
				}
			}

			// Player...
			APlayableCharacter* Playable = Cast<APlayableCharacter>(Player);
			if (Playable)
			{
				GameData->PlayerData = FActorData(Playable);
			}

			// Bots...
			TArray<AActor*> Bots;
			UGameplayStatics::GetAllActorsOfClass(CurrentWorld, ABotCharacter::StaticClass(), Bots);
			int32 BotsCount = Bots.Num();
			UE_LOG(LogTemp, Display, TEXT("Found %d bots"), BotsCount);
			for (TActorIterator<ABotCharacter> It(CurrentWorld); It; ++It)
			{
				// Indices matter, insert at ActorID index
				//GameData->BotData.Insert(FActorData(*It), It->GetID());
				GameData->BotData.Add(It->GetID(), FActorData(*It));
			}
			//for (int32 j = 0; j < Bots.Num(); j++)
			//{
			//	// Upcast to playable as all bots derive from playable
			//	APlayableCharacter* Bot = Cast<APlayableCharacter>(Bots[j]);
			//	if (Bot) { GameData->BotData.Add(FActorData(Bot)); }
			//}
		}

		// Save process
		if (bUseAsync)
		{
			FAsyncSaveGameToSlotDelegate SavedDelegate;
			SavedDelegate.BindUObject(this, &UThesisGameInstance::SaveDone);
			UGameplayStatics::AsyncSaveGameToSlot(GameData, SlotName, Index, SavedDelegate);
			// Lock consequent save requests
			PendingSave = true;
		}
		else
		{
			return UGameplayStatics::SaveGameToSlot(GameData, SaveSlot, SaveIndex);
		}
	}
	return false;
}

void UThesisGameInstance::SaveDone(const FString& SlotName, const int32 Index, bool bSuccess)
{
	if (bSuccess)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Save completed!"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Save failed."));
	}
	// Unlock for saving again
	PendingSave = false;
}

class UThesisSaveGame* UThesisGameInstance::LoadGame(bool bUseAsync)
{
	if (!SaveSlot.IsEmpty() && SaveIndex > -1 && UGameplayStatics::DoesSaveGameExist(SaveSlot, SaveIndex))
	{
		if (bUseAsync)
		{
			FAsyncLoadGameFromSlotDelegate LoadedDelegate;
			LoadedDelegate.BindUObject(this, &UThesisGameInstance::LoadDone);
			UGameplayStatics::AsyncLoadGameFromSlot(SaveSlot, SaveIndex, LoadedDelegate);
			PendingLoad = true;
		}
		else
		{
			GameData = Cast<UThesisSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, SaveIndex));
			LoadFillData(GameData);
			return GameData;
		}
	}
	return NULL;
}

class UThesisSaveGame* UThesisGameInstance::LoadGame(const FString& SlotName, const int32 UserIndex, bool bUseAsync)
{
	// Load process
	if (!SlotName.IsEmpty() && UserIndex > -1 && UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		if (bUseAsync)
		{
			FAsyncLoadGameFromSlotDelegate LoadedDelegate;
			LoadedDelegate.BindUObject(this, &UThesisGameInstance::LoadDone);
			UGameplayStatics::AsyncLoadGameFromSlot(SlotName, UserIndex, LoadedDelegate);
			PendingLoad = true;
		}
		else
		{
			GameData = Cast<UThesisSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
			LoadFillData(GameData);
			return GameData;
		}
	}
	return NULL;
}

void UThesisGameInstance::LoadDone(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	GameData = Cast<UThesisSaveGame>(LoadedGameData);
	if (GameData)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Load completed!"));
		// Unlock for saving again
		PendingLoad = false;
		//UE_LOG(LogTemp, Display, TEXT("Loaded level: %s"), *GameData->LastLevel.ToString());
		//UE_LOG(LogTemp, Display, TEXT("Player location %d,%d,%d"), GameData->PlayerLocation.X, GameData->PlayerLocation.Y, GameData->PlayerLocation.Z);
		LoadFillData(GameData);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Load failed."));
	}
}

void UThesisGameInstance::LoadFillData(class UThesisSaveGame* LoadedGameData)
{
	if (LoadedGameData)
	{
		bIsLoadedGame = true;

		// Determine where to spawn
		if (!LoadedGameData->LastCheckpoint.IsNone())
		{
			CheckpointPlayerStartName = LoadedGameData->LastCheckpoint;
		}
		
		// Open last level
		UGameplayStatics::OpenLevel(this, LoadedGameData->LastLevel, true);
	}
}

bool UThesisGameInstance::IsPendingSave() const
{
	return PendingSave;
}

bool UThesisGameInstance::IsPendingLoad() const
{
	return PendingLoad;
}

bool UThesisGameInstance::IsLoadedGame() const
{
	return bIsLoadedGame;
}

class UThesisSaveGame* UThesisGameInstance::GetGameData() const
{
	return GameData;
}