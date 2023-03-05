// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Quests/Quest.h"
#include "QuestTrigger.generated.h"

/**
 * 
 */
UCLASS()
class THESISGAME_API AQuestTrigger : public ATriggerBox
{
	GENERATED_UCLASS_BODY()

	virtual void PostInitializeComponents() override;

public:
	UFUNCTION()
	void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger Default")
	class TSubclassOf<AQuest> TargetQuestClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger Default")
	int32 TargetStage;

	//By default, only player is able to interact with this type of trigger
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger Default")
	class AActor* TriggeredBy;
protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	AQuest* TargetQuest;

	UFUNCTION()
	void RegisterTargetQuest();
};
