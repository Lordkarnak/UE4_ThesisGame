// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Blueprint/UserWidget.h"
#include "Quest.generated.h"

class AObjective;
class UAudioComponent;
class USoundCue;

UENUM()
namespace EQuestStates
{
	enum Type
	{
		Inactive,
		Started,
		Completed,
		Failed
	};
}

USTRUCT(Blueprintable)
struct FQuestObjective
{
	GENERATED_USTRUCT_BODY();

public:
	/** The name of the objective. Displayed under the objectives list of the quest log and on screen */
	UPROPERTY (EditDefaultsOnly, BlueprintReadWrite, Category = Header)
	FName ObjectiveName;

	/** A more in-depth description of the objective. This field is displayed under the detailed section of the quest log. */
	UPROPERTY (EditDefaultsOnly, BlueprintReadWrite, Category = Header)
	FText ObjectiveDescription;

	/** Is this objective mandatory to progress through the quest? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Properties)
	bool bIsMandatory;

	/** Quest stage this objective is linked to. That way, when the quest is updated to a stage, an Objective is automatically started and all others are completed or failed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Properties)
	int32 iLinkedStage;

	UPROPERTY(BlueprintReadOnly, Category = Properties)
	bool bIsStarted;

	UPROPERTY(BlueprintReadOnly, Category = Properties)
	bool bIsActive;

private:
	/** The objective state. Can be anything between Started, Completed, Failed */
	UPROPERTY (Transient)
	TEnumAsByte<EQuestStates::Type> ObjectiveState;
	
public:
	FQuestObjective()
	{
		ObjectiveName = NAME_None;
		ObjectiveDescription = FText();
		bIsMandatory = false;
		iLinkedStage = 0;
		bIsStarted = false;
		bIsActive = false;
		ObjectiveState = EQuestStates::Inactive;
	}

	bool operator == (const FQuestObjective& Obj2) const
	{
		return Obj2.ObjectiveName == ObjectiveName;
	}

	//UFUNCTION (BlueprintCallable)
	bool Start()
	{
		if (ObjectiveState == EQuestStates::Inactive)
		{
			ObjectiveState = EQuestStates::Started;
			bIsStarted = true;
			return true;
		}
		return false;
	}

	//UFUNCTION (BlueprintCallable)
	bool Activate()
	{
		if (ObjectiveState == EQuestStates::Started)
		{
			bIsActive = true;
			return true;
		}
		return false;
	}

	//UFUNCTION(BlueprintCallable)
	bool Deactivate()
	{
		if (ObjectiveState == EQuestStates::Started && bIsActive)
		{
			bIsActive = false;
			return true;
		}
		return false;
	}

	//UFUNCTION(BlueprintCallable)
	bool End(bool bFail)
	{
		if (ObjectiveState == EQuestStates::Started)
		{
			if (bFail)
			{
				ObjectiveState = EQuestStates::Failed;
			}
			else
			{
				ObjectiveState = EQuestStates::Completed;
			}
			bIsActive = false;
			return true;
		}
		return false;
	}

	bool isStarted() { return (ObjectiveState == EQuestStates::Started); }

	bool isCompleted() { return (ObjectiveState == EQuestStates::Completed); }
};

/**
 * A single quest object that holds objectives and manipulates the quest widget
 */
UCLASS(Blueprintable)
class THESISGAME_API AQuest : public AInfo
{
	GENERATED_BODY()
	
public:
	AQuest();

	UFUNCTION(BlueprintCallable)
	void Start();

	UFUNCTION(BlueprintCallable)
	void ToggleQuest(bool bActive);

	UFUNCTION(BlueprintCallable)
	bool Update(int32 NewStage);

	UFUNCTION(BlueprintCallable)
	void Complete();

	UFUNCTION(BlueprintCallable)
	void ToggleObjective(FQuestObjective& WhichObjective, bool bActivate);

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentStage() const;

public:
	//***********
	// Quest related properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText QuestDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 StartStage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 EndStage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	USoundCue* StartupSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FQuestObjective> ObjectivesList;

	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FQuestObjective> CurrentObjectives;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bAutoActivate;

	//***********
	// Quest related helper functions
	UFUNCTION(BlueprintCallable)
	bool IsStarted() const;

	UFUNCTION(BlueprintCallable)
	bool IsActive() const;

	UFUNCTION(BlueprintCallable)
	bool IsCompleted() const;

	UFUNCTION(BlueprintCallable)
	bool CanUpdate(int32 Stage) const;

protected:
	EQuestStates::Type CurrentQuestState;

	int32 CurrentStage;

	bool bIsActive;
	bool PendingUpdate;

	UFUNCTION()
	void DetermineQuestFlow();
};
