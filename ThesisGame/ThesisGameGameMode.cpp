// Copyright Epic Games, Inc. All Rights Reserved.

#include "ThesisGameGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "ThesisGameGameState.h"
#include "Characters/PlayableController.h"
#include "Characters/PlayableCharacter.h"
#include "ThesisSaveGame.h"
#include "ThesisGameHUD.h"
#include "ThesisGameInstance.h"
#include "Quests/Quest.h"

AThesisGameGameMode::AThesisGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/ThesisGame/Blueprints/Characters/PlayerBP"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	HUDClass = AThesisGameHUD::StaticClass();
	PlayerControllerClass = APlayableController::StaticClass();
	GameStateClass = AThesisGameGameState::StaticClass();
}

void AThesisGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	//Set a pointer to the game instance
	UWorld* World = GetWorld();
	if (World)
	{
		CurrentGameInstance = Cast<UThesisGameInstance>(UGameplayStatics::GetGameInstance(World));
	}

	//Bind our Player died delegate to the Gamemode's PlayerDied function.
	if (!bPlayerDied.IsBound())
	{
		bPlayerDied.AddDynamic(this, &AThesisGameGameMode::OnPlayerDeath);
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		AThesisGameHUD* PlayerHUD = PlayerController->GetHUD<AThesisGameHUD>();
		if (PlayerHUD != nullptr)
		{
			PlayerHUD->ChangeMenuWidget(StartingWidgetClass);
		}
	}

	//Spawn quests and start them
	if (CurrentGameInstance && CurrentGameInstance->IsLoadedGame())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AThesisGameGameMode::SpawnLoadedQuestList);
		GetWorldTimerManager().SetTimerForNextTick(this, &AThesisGameGameMode::StoreLoadedBots);
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AThesisGameGameMode::SpawnDefaultQuestList);
		GetWorldTimerManager().SetTimerForNextTick(this, &AThesisGameGameMode::StoreDefaultBots);
	}
}

class AActor* AThesisGameGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UE_LOG(LogTemp, Display, TEXT("ChoosePlayerStart triggered"));
	if (Player)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			UThesisGameInstance* GameInstance = Cast<UThesisGameInstance>(UGameplayStatics::GetGameInstance(World));
			if (GameInstance)
			{
				TArray<class AActor*> PlayerStarts;
				UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
				// Search for checkpoints
				for (TActorIterator<APlayerStart> It(World); It; ++It)
				{
					if (GameInstance->CheckpointPlayerStartName == It->PlayerStartTag)
					{
						UE_LOG(LogTemp, Display, TEXT("Checkpoint (%s) selected"), *GameInstance->CheckpointPlayerStartName.ToString());
						return *It;
					}
				}

				// Checkpoint not found, search for default start
				for (TActorIterator<APlayerStart> It(World); It; ++It)
				{
					if (GameInstance->FirstPlayerStartName == It->PlayerStartTag)
					{
						UE_LOG(LogTemp, Display, TEXT("First player start (%s) selected"), *GameInstance->FirstPlayerStartName.ToString());
						return *It;
					}
				}
			}
		}
	}

	// Default start not found, default player start selected
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AThesisGameGameMode::StartPlay()
{
	Super::StartPlay();

	check(GEngine != nullptr);

	//Display debug message
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Thesis Game Mode running."));
}

void AThesisGameGameMode::OnPlayerDeath(ACharacter* CallerCharacter)
{
	UE_LOG(LogTemp, Display, TEXT("OnPlayerDeath triggered"));
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	ABotCharacter* Bot = Cast<ABotCharacter>(CallerCharacter);
	// Is caller a player?
	if (CallerCharacter == Player)
	{
		FTimerHandle PlayerRespawnTimer;
		GetWorldTimerManager().SetTimer(PlayerRespawnTimer, this, &AThesisGameGameMode::ReloadGame, 5.0f, false);
	}
	// Is caller a bot?
	else if (Bot != NULL)
	{
		AThesisGameGameState* CurrentState = GetGameState<AThesisGameGameState>();
		if (CurrentState != nullptr)
		{
			CurrentState->AddDeadBot(Bot);
		}
	}
}

void AThesisGameGameMode::ReloadGame()
{
	//RestartPlayer(this);
	// Simply reopen the already opened level, resets everything
	UThesisGameInstance* GameInstance = Cast<UThesisGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (GameInstance && GameInstance->IsLoadedGame())
	{
		GameInstance->LoadGame();
	}
	UGameplayStatics::OpenLevel(this, GetWorld()->GetFName(), true);

	//RestartPlayer(Controller);
	/*if (GetLocalRole() == ROLE_Authority)
	{
		AController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (Controller)
		{
			UE_LOG(LogTemp, Display, TEXT("ReloadGame triggered"));
			TArray<AActor*> SpawnPoints;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);
			if (SpawnPoints.Num() > 0 && SpawnPoints[0] != nullptr)
			{
				APawn* Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, SpawnPoints[0]->GetActorLocation(), FRotator::ZeroRotator);
				Controller->Possess(Pawn);
			}
		}
	}*/
}

void AThesisGameGameMode::SpawnLoadedQuestList()
{
	UThesisSaveGame* GameData = CurrentGameInstance->GetGameData();
	UWorld* CurrentWorld = GetWorld();
	AThesisGameGameState* CurrentState = CurrentWorld->GetGameState<AThesisGameGameState>();
	if (GameData && CurrentWorld && CurrentState)
	{
		for (const FQuestData& QuestData : GameData->CurrentQuestsData)
		{
			if (QuestData.QuestClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AQuest* QuestInstance = CurrentWorld->SpawnActor<AQuest>(QuestData.QuestClass, SpawnParams);
				QuestInstance->ToggleQuest(QuestData.IsActive);
				QuestInstance->Update(QuestData.CurrentStage);
				CurrentState->AddQuest(QuestInstance);
				UE_LOG(LogTemp, Display, TEXT("Loaded quest (%s)"), *QuestData.QuestClass->GetName());
			}
		}
	}

	// No quests saved, just start default quests
	if (CurrentState && CurrentState->GetCurrentQuests().Num() < 1)
	{
		SpawnDefaultQuestList();
	}
}

void AThesisGameGameMode::SpawnDefaultQuestList()
{
	int32 NumQuests = DefaultQuests.Num();
	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && NumQuests > 0)
	{
		AThesisGameGameState* CurrentState = CurrentWorld->GetGameState<AThesisGameGameState>();
		if (CurrentState)
		{
			for (int32 i = 0; i < NumQuests; i++)
			{
				if (DefaultQuests[i])
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					AQuest* QuestInstance = CurrentWorld->SpawnActor<AQuest>(DefaultQuests[i], SpawnParams);
					CurrentState->AddQuest(QuestInstance);
				}
			}
		}
	}
}

void AThesisGameGameMode::StoreLoadedBots()
{
	UThesisSaveGame* GameData = CurrentGameInstance->GetGameData();
	UWorld* CurrentWorld = GetWorld();
	AThesisGameGameState* CurrentState = GetGameState<AThesisGameGameState>();
	if (GameData && CurrentWorld && CurrentState)
	{
		TArray<AActor*> Bots;
		UGameplayStatics::GetAllActorsOfClass(CurrentWorld, ABotCharacter::StaticClass(), Bots);
		for (AActor* BotActor : Bots)
		{
			ABotCharacter* BotCharacter = Cast<ABotCharacter>(BotActor);
			if (BotCharacter != nullptr)
			{
				int32 id = BotCharacter->GetID();
				if (GameData->BotData.Contains(id))
				{
					if (GameData->BotData[id].CurrentHealth > 0)
					{
						CurrentState->AddBot(BotCharacter);
						UE_LOG(LogTemp, Display, TEXT("Added loaded alive bot (%d)"), BotCharacter->GetID());
					}
					else
					{
						CurrentState->AddDeadBot(BotCharacter);
						BotCharacter->Destroy();
						UE_LOG(LogTemp, Display, TEXT("Added loaded dead bot (%d)"), BotCharacter->GetID());
					}
				}
			}
		}
	}
}

void AThesisGameGameMode::StoreDefaultBots()
{
	UWorld* CurrentWorld = GetWorld();
	AThesisGameGameState* CurrentState = GetGameState<AThesisGameGameState>();
	if (CurrentWorld && CurrentState)
	{
		TArray<AActor*> Bots;
		UGameplayStatics::GetAllActorsOfClass(CurrentWorld, ABotCharacter::StaticClass(), Bots);
		for (AActor* BotActor : Bots)
		{
			ABotCharacter* BotCharacter = Cast<ABotCharacter>(BotActor);
			if (BotCharacter != nullptr)
			{
				if (BotCharacter->IsAlive())
				{
					CurrentState->AddBot(BotCharacter);
					UE_LOG(LogTemp, Display, TEXT("Added alive bot (%d)"), BotCharacter->GetID());
				}
				else
				{
					CurrentState->AddDeadBot(BotCharacter);
					UE_LOG(LogTemp, Display, TEXT("Added dead bot (%d)"), BotCharacter->GetID());
				}
			}
		}
	}
}

bool AThesisGameGameMode::IsVibrationEnabled()
{
	return bIsVibrationEnabled;
}

//AThesisGameHUD* AThesisGameGameMode::GetPlayerHUD() const
//{
//	return PlayerHUD;
//}