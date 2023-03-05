// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Quests/Quest.h"
#include "ThesisGameGameState.generated.h"

//class AQuest;

/**
 * 
 */
UCLASS()
class THESISGAME_API AThesisGameGameState : public AGameStateBase
{
	GENERATED_BODY()

	AThesisGameGameState();

public:
	UFUNCTION(BlueprintCallable, Category = Quests)
	bool AddQuest(AQuest* NewQuest);

	UFUNCTION(BlueprintCallable, Category = Quests)
	bool RemoveQuest(AQuest* WhichQuest, bool bForceRemove);

	UFUNCTION(BlueprintCallable, Category = Quests)
	AQuest* GetActiveQuest() const;

	UFUNCTION(BlueprintCallable, Category = Quests)
	void SetActiveQuest(AQuest* WhichQuest);

	UFUNCTION(BlueprintCallable, Category = Quests)
	TArray<AQuest*> GetCurrentQuests() const;
	
	UFUNCTION(BlueprintCallable, Category = Quests)
	TArray<AQuest*> GetCompletedQuests() const;

	UFUNCTION(BlueprintCallable, Category = Quests)
	AQuest* FindQuest(class TSubclassOf<AQuest> WhichQuestClass);

private:
	UPROPERTY(Transient)
	AQuest* ActiveQuest;

	UPROPERTY(Transient)
	TArray<AQuest*> CurrentQuestList;

	UPROPERTY(Transient)
	TArray<AQuest*> CompletedQuestList;

	UPROPERTY(Transient)
	TArray<FName> QuestList;
	
};
