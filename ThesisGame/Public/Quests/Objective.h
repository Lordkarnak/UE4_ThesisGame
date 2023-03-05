// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Objective.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class THESISGAME_API AObjective : public AInfo
{
	GENERATED_BODY()
	
public:
	AObjective();

	void Start();

	void Finish(bool bFailed);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bIsMandatory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText ObjectiveDescription;

	UFUNCTION(BlueprintCallable)
	bool IsActive();

	UFUNCTION(BlueprintCallable)
	bool IsCompleted();

	/** Quest stage this objective is linked to. That way, when the quest is updated to a stage, an Objective is automatically started and all others are completed or failed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Properties)
	int32 iLinkedStage;

protected:
	bool bIsRunning;

	bool bIsCompleted;

	bool bIsFailed;
};
