// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameStructs.h"
#include "ThesisSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class THESISGAME_API UThesisSaveGame : public USaveGame
{
	GENERATED_BODY()
	
    public:
        UPROPERTY(VisibleAnywhere, Category = Basic)
        FString SaveSlotName;

        UPROPERTY(VisibleAnywhere, Category = Basic)
        uint32 UserIndex;

        UPROPERTY(BlueprintReadOnly)
        FName LastLevel;

        UPROPERTY(BlueprintReadOnly)
        FName LastCheckpoint;

        UPROPERTY(BlueprintReadOnly, Category = Stats)
        int32 Kills;

        UPROPERTY(BlueprintReadOnly, Category = Stats)
        int32 Deaths;

        UPROPERTY()
        struct FQuestData ActiveQuestData;

        UPROPERTY()
        TArray<struct FQuestData> CurrentQuestsData;

        UPROPERTY()
        TArray<struct FQuestData> CompletedQuestsData;

        UPROPERTY()
        struct FActorData PlayerData;

        UPROPERTY()
        TMap<int32, struct FActorData> BotData;

        UThesisSaveGame();
};