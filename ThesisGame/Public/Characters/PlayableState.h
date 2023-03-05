// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayableState.generated.h"

/**
 * 
 */
UCLASS()
class THESISGAME_API APlayableState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	int32 GetNumKills() const;

	UFUNCTION(BlueprintCallable)
	int32 GetNumDeaths() const;

	UFUNCTION(BlueprintCallable)
	int32 GetNumQuestsTaken() const;

	UFUNCTION(BlueprintCallable)
	void AddKill();

	UFUNCTION(BlueprintCallable)
	void AddDeath();

	UFUNCTION(BlueprintCallable)
	void AddQuestTaken();

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Player Statistics")
	int32 Kills;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Player Statistics")
	int32 Deaths;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Player Statistics")
	int32 QuestsTaken;
};
