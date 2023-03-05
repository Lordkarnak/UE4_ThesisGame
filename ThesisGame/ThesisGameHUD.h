// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "Quests/Quest.h"
#include "ThesisGameHUD.generated.h"

//class AQuest;

UENUM(BlueprintType)
namespace EInteractionTypes
{
	enum Type
	{
		Activate UMETA(DisplayName="Activate"),
		Push UMETA(DisplayName = "Push"),
		Take UMETA(DisplayName = "Take"),
		Open UMETA(DisplayName = "Open"),
		Close UMETA(DisplayName = "close")
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
	void EnableQuestStartedWidget();

	UFUNCTION(BlueprintCallable)
	void AddToStartQueue(AQuest* WhichQuest);

	UFUNCTION(BlueprintCallable)
	TArray<AQuest*> GetStartedQuests() const;

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

	/** A quest started widget instance */
	UUserWidget* QuestStartedWidget;

	/** An instance of the current interaction */
	EInteractionTypes::Type CurrentInteraction;

	/** String representation of the current interaction */
	FText CurrentInteractionText;

	/** Pointer to selected quest in menu */
	AQuest* InMenuQuest;

	/** Array of freshly started quests*/
	TArray<AQuest*> StartedQuestsQueue;

	UFUNCTION()
	void RemoveFromStartQueue(AQuest* WhichQuest);

	UFUNCTION()
	void MarkWidgetForDeletion(UUserWidget* WhichWidget);
};

