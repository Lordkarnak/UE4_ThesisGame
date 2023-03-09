// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "ThesisGameGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDeathSignature, ACharacter*, Character);

class UThesisGameInstance;
struct FQuestData;

UCLASS(minimalapi)
class AThesisGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AThesisGameGameMode();

	const FOnPlayerDeathSignature& GetOnPlayerDeath() const { return bPlayerDied; }

	UFUNCTION(BlueprintCallable, Category = Controller)
	bool IsVibrationEnabled();

	/*UFUNCTION(BlueprintCallable, Category = "UI")
	AThesisGameHUD* GetPlayerHUD() const;*/

	/* Define the mission log widget class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> LogWidgetClass;

	/** Main player HUD, contains health, crosshair and ammo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDWidget;

	/** Interaction MSG widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> InteractionWidget;

	/** Quest started MSG widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> QuestMSGWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> ObjectiveMSGWidgetClass;

	/** Define the starting widget class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> StartingWidgetClass;

protected:
	/** Called on game start */
	virtual void BeginPlay() override;

	/** Called on gameplay start */
	virtual void StartPlay() override;

	virtual class AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual bool ShouldSpawnAtStartSpot(AController* Player) override { return false; }

	/** Event that gets broadcasted when the player pawn dies */
	UFUNCTION()
	virtual void OnPlayerDeath(ACharacter* Character);

	/** Called when the game needs to be reloaded from save or from start */
	virtual void ReloadGame();

	UFUNCTION()
	void SpawnLoadedQuestList();

	UFUNCTION()
	void SpawnDefaultQuestList();

	//Signature to bind delegate. 
	UPROPERTY()
	FOnPlayerDeathSignature bPlayerDied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Controller)
	uint8 bIsVibrationEnabled : 1;

	/** Default quest list */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Quests)
	TArray<TSubclassOf<class AQuest>> DefaultQuests;

private:
	/** Pointer to the current game instance, used to access game data */
	UThesisGameInstance* CurrentGameInstance;
};