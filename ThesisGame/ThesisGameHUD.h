// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "Quests/Quest.h"
#include "ThesisGameHUD.generated.h"

UENUM(BlueprintType)
namespace EInteractionTypes
{
	enum Type
	{
		Activate UMETA(DisplayName="activate"),
		Push UMETA(DisplayName = "push"),
		Take UMETA(DisplayName = "take"),
		Open UMETA(DisplayName = "open"),
		Close UMETA(DisplayName = "close"),
		Save UMETA(DisplayName = "save progress")
	};
}

UCLASS()
class AThesisGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	AThesisGameHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable)
	void EnableInteractionWidget(USceneComponent* ParentComp);

	UFUNCTION(BlueprintCallable)
	void DisableInteractionWidget();

	UFUNCTION(BlueprintCallable)
	void EnableQuestLogWidget();

	UFUNCTION(BlueprintCallable)
	void DisableQuestLogWidget();

	UFUNCTION(BlueprintCallable)
	void ChangeMenuWidget(TSubclassOf<UUserWidget> NewClassWidget);

	UFUNCTION(BlueprintCallable)
	void DetermineInteraction(UPrimitiveComponent* TargetComp, AActor* TargetActor);

	UFUNCTION(BlueprintCallable)
	EInteractionTypes::Type GetCurrentInteractionEnum() const;

	UFUNCTION(BlueprintCallable)
	FText GetCurrentInteractionText() const;

	UFUNCTION(BlueprintCallable)
	void SetInMenuActiveQuest(AQuest* NewQuest);

	UFUNCTION(BlueprintCallable)
	AQuest* GetInMenuActiveQuest() const;

	UFUNCTION(BlueprintCallable)
	void ShowMSGWidget(TSubclassOf<class UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable)
	void AddQuestToQueue(AQuest* WhichQuest);

	UFUNCTION(BlueprintCallable)
	void AddObjectiveToQueue(const FQuestObjective& Objective);

	UFUNCTION(BlueprintCallable)
	TArray<AQuest*> GetQuestsQueue() const;

	UFUNCTION(BlueprintCallable)
	TArray<FQuestObjective> GetObjectivesQueue() const;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	/** A HUD interaction widget instance */
	UUserWidget* HUDInteractionWidget;
	
	/** A HUD component instance */
	UWidgetComponent* HUDWidgetComponent;

	/** A Quest log widget instance */
	UUserWidget* HUDQuestLogWidget;

	/** A menu widget instance */
	UUserWidget* CurrentMenuWidget;

	/** A message widget instance, shown only temporarily */
	UUserWidget* MSGWidget;

	/** An instance of the current interaction */
	EInteractionTypes::Type CurrentInteraction;

	/** String representation of the current interaction */
	FText CurrentInteractionText;

	/** Pointer to selected quest in menu */
	AQuest* InMenuQuest;

	/** Array of freshly started quests*/
	TArray<AQuest*> QuestsQueue;

	TArray<FQuestObjective> ObjectivesQueue;

	UFUNCTION()
	void RemoveFromQuestQueue(AQuest* WhichQuest);

	UFUNCTION()
	void RemoveFromObjectiveQueue(const FQuestObjective& Objective);

	UFUNCTION()
	void MarkWidgetForDeletion(UUserWidget* WhichWidget);
};

