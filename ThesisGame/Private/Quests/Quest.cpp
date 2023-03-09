// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest.h"
#include "Objective.h"
#include "Kismet/GameplayStatics.h"
#include "ThesisGameGameState.h"
#include "ThesisGameGameMode.h"
#include "ThesisGameHUD.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

//Sets starting parameters for quest
AQuest::AQuest()
{
	QuestName = FName(NAME_None);
	QuestDescription = FText();

	CurrentQuestState = EQuestStates::Inactive;
	CurrentStage = -1;

	StartStage = 0;
	EndStage = 100;

	DetermineQuestFlow();
}

void AQuest::Start()
{
	if (!IsStarted())
	{
		CurrentQuestState = EQuestStates::Started;

		if (StartupSound != nullptr)
		{
			APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
			UGameplayStatics::SpawnSoundAttached((USoundBase*) StartupSound, PlayerPawn->GetRootComponent());
		}

		// Use a local timer handle
		//Update(5);

		//Display debug message
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Quest %s started."), *QuestName.ToString()));
		if (bAutoActivate)
		{
			ToggleQuest(true);
		}

		// Just ask from HUD to show the "quest started" message
		APlayerController* CurrentPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (CurrentPlayerController)
		{
			AThesisGameHUD* PlayerHUD = CurrentPlayerController->GetHUD<AThesisGameHUD>();
			AThesisGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
			if (PlayerHUD && GameMode)
			{
				PlayerHUD->AddQuestToQueue(this);
				PlayerHUD->ShowMSGWidget(GameMode->QuestMSGWidgetClass);
			}
		}
	}
}

void AQuest::ToggleQuest(bool bActive)
{
	if (IsStarted())
	{
		UWorld* CurrentWorld = GetWorld();
		if (CurrentWorld)
		{
			AThesisGameGameState* CurrentGameState = CurrentWorld->GetGameState<AThesisGameGameState>();
			if (CurrentGameState)
			{
				if (bActive && CurrentGameState->GetActiveQuest() != this)
				{
					CurrentGameState->SetActiveQuest(this);
					bIsActive = true;
				}
				else if (!bActive)
				{
					CurrentGameState->SetActiveQuest(nullptr);
					bIsActive = false;
				}
			}
		}
	}
}

bool AQuest::Update(int32 NewStage)
{
	if (CanUpdate(NewStage))
	{
		APlayerController* CurrentPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		AThesisGameHUD* PlayerHUD = nullptr;
		if (CurrentPlayerController)
		{
			PlayerHUD = CurrentPlayerController->GetHUD<AThesisGameHUD>();
		}
		// End current Objectives linked to CurrentStage
		for (FQuestObjective& Objective : CurrentObjectives)
		{
			if (Objective.iLinkedStage == CurrentStage)
			{
				Objective.End();
				PlayerHUD->AddObjectiveToQueue(Objective);
			}
		}
		// Update list of current objectives
		for (FQuestObjective& Objective : ObjectivesList)
		{
			if (Objective.iLinkedStage == NewStage)
			{
				Objective.Start();
				CurrentObjectives.AddUnique(Objective);
				PlayerHUD->AddObjectiveToQueue(Objective);
				//Display debug message
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Quest objective %s started."), *ObjectivesList[i].ObjectiveName.ToString()));
			}
		}
		// Update stage
		PendingUpdate = true;
		CurrentStage = NewStage;
		DetermineQuestFlow();
		PendingUpdate = false;

		// Ask to display objective message widget
		AThesisGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
		if (PlayerHUD && GameMode)
		{
			PlayerHUD->ShowMSGWidget(GameMode->ObjectiveMSGWidgetClass);
		}
		return true;
	}
	return false;
}

void AQuest::Complete()
{
	CurrentQuestState = EQuestStates::Completed;
	CurrentStage = EndStage;
	// Just ask from HUD to show the "quest ended" message
	APlayerController* CurrentPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (CurrentPlayerController)
	{
		AThesisGameHUD* PlayerHUD = CurrentPlayerController->GetHUD<AThesisGameHUD>();
		AThesisGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
		if (PlayerHUD && GameMode)
		{
			PlayerHUD->ShowMSGWidget(GameMode->QuestMSGWidgetClass);
		}
	}
}

void AQuest::ToggleObjective(FQuestObjective& WhichObjective, bool bActivate)
{
	if (ObjectivesList.Contains(WhichObjective))
	{
		if (bActivate)
		{
			CurrentObjectives.AddUnique(WhichObjective);
			WhichObjective.Activate();
		}
		else
		{
			CurrentObjectives.Remove(WhichObjective);
			WhichObjective.Deactivate();
		}
	}
}

int32 AQuest::GetCurrentStage() const
{
	return CurrentStage;
}

bool AQuest::StartObjective(FName TargetObjectiveName)
{
	AThesisGameHUD* PlayerHUD = nullptr;
	APlayerController* CurrentPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AThesisGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
	if (CurrentPlayerController)
	{
		PlayerHUD = CurrentPlayerController->GetHUD<AThesisGameHUD>();
	}
	for (FQuestObjective& Objective : CurrentObjectives)
	{
		if (Objective.ObjectiveName == TargetObjectiveName && Objective.iLinkedStage == CurrentStage)
		{
			Objective.Start();
			// Ask to display objective message widget
			if (PlayerHUD && GameMode)
			{
				PlayerHUD->AddObjectiveToQueue(Objective);
				PlayerHUD->ShowMSGWidget(GameMode->ObjectiveMSGWidgetClass);
			}
			return true;
		}
	}
	return false;
}

bool AQuest::CompleteObjective(FName TargetObjectiveName)
{
	AThesisGameHUD* PlayerHUD = nullptr;
	APlayerController* CurrentPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	AThesisGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
	if (CurrentPlayerController)
	{
		PlayerHUD = CurrentPlayerController->GetHUD<AThesisGameHUD>();
	}
	for (FQuestObjective& Objective : CurrentObjectives)
	{
		if (Objective.ObjectiveName == TargetObjectiveName && Objective.iLinkedStage == CurrentStage)
		{
			Objective.End();
			// Ask to display objective message widget
			if (PlayerHUD && GameMode)
			{
				PlayerHUD->AddObjectiveToQueue(Objective);
				PlayerHUD->ShowMSGWidget(GameMode->ObjectiveMSGWidgetClass);
			}
			return true;
		}
	}
	return false;
}

bool AQuest::CanUpdate(int32 TestStage) const
{
	bool PreviousCompleted = true;
	if (TestStage > CurrentStage && !IsCompleted())
	{
		if (CurrentObjectives.Num() > 0)
		{
			for (auto Objective : CurrentObjectives)
			{
				PreviousCompleted = PreviousCompleted && (Objective.isCompleted() || Objective.bIsMandatory == false);
			}
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("Can Quest Stage update? (%s)"), *FString(PreviousCompleted ? "True": "False"));
	return PreviousCompleted;
}

bool AQuest::IsStarted() const
{
	return (CurrentQuestState == EQuestStates::Started && CurrentStage >= StartStage);
}

bool AQuest::IsActive() const
{
	return IsStarted() && bIsActive;
}

bool AQuest::IsCompleted() const
{
	return (CurrentQuestState == EQuestStates::Completed) && (CurrentStage >= EndStage);
}

void AQuest::DetermineQuestFlow()
{
	if (!IsStarted())
	{
		if (bAutoActivate || CurrentStage > StartStage || PendingUpdate)
		{
			Start();
		}
	}
	// Complete the quest if necessary
	else if (IsStarted() && CurrentStage >= EndStage)
	{
		Complete();
	}
}