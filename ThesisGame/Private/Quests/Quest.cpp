// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest.h"
#include "Objective.h"
#include "Kismet/GameplayStatics.h"
#include "ThesisGameGameState.h"
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
		if (CurrentPlayerController != nullptr)
		{
			AThesisGameHUD* PlayerHUD = CurrentPlayerController->GetHUD<AThesisGameHUD>();
			if (PlayerHUD != nullptr)
			{
				PlayerHUD->AddToStartQueue(this);
				PlayerHUD->EnableQuestStartedWidget();
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
		// End current Objectives linked to CurrentStage
		if (CurrentObjectives.Num() > 0)
		{
			for (int32 i = 0; i < CurrentObjectives.Num(); i++)
			{
				if (CurrentObjectives[i].iLinkedStage == CurrentStage)
				{
					CurrentObjectives[i].End(false);
				}
			}
		}
		// Update list of current objectives
		if (ObjectivesList.Num() > 0)
		{
			for (int32 i = 0; i < ObjectivesList.Num(); i++)
			{
				if (&ObjectivesList[i] && ObjectivesList[i].iLinkedStage == NewStage)
				{
					ObjectivesList[i].Start();
					CurrentObjectives.AddUnique(ObjectivesList[i]);
					//Display debug message
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Quest objective %s started."), *ObjectivesList[i].ObjectiveName.ToString()));
				}
			}
		}
		// Update stage
		PendingUpdate = true;
		CurrentStage = NewStage;
		DetermineQuestFlow();
		PendingUpdate = false;
		return true;
	}
	return false;
}

void AQuest::Complete()
{
	CurrentQuestState = EQuestStates::Completed;
	CurrentStage = EndStage;
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
	UE_LOG(LogTemp, Warning, TEXT("Can Quest Stage update? (%s)"), *FString(PreviousCompleted ? "True": "False"));
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