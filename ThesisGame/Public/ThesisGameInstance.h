// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ThesisGameInstance.generated.h"

class ASaveTerminal;
class UThesisSaveGame;

/**
 * 
 */
UCLASS()
class THESISGAME_API UThesisGameInstance: public UGameInstance
{
	GENERATED_BODY()

	void Init() override;

	const FString SaveSlot = "SaveSlot1";
	
	int32 SaveIndex = 0;

public:

	UFUNCTION(BlueprintCallable, Category = Game)
	bool SaveGame(ASaveTerminal* Caller, const FString& SlotName, const int32 UserIndex, bool bUseAsync = false);

	class UThesisSaveGame* LoadGame(bool bUseAsync = false);

	UFUNCTION(BlueprintCallable, Category = Game)
	class UThesisSaveGame* LoadGame(const FString& SlotName, const int32 UserIndex, bool bUseAsync = false);

	UFUNCTION(BlueprintCallable, Category = Game)
	void LoadAutosave(const FString& SlotName, const int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category = Game)
	bool IsPendingSave() const;

	UFUNCTION(BlueprintCallable, Category = Game)
	bool IsPendingLoad() const;

	UFUNCTION(BlueprintCallable, Category = Game)
	bool IsLoadedGame() const;

	UFUNCTION()
	class UThesisSaveGame* GetGameData() const;

	FName FirstPlayerStartName;

	FName CheckpointPlayerStartName;

protected:
	UPROPERTY()
	class UThesisSaveGame* GameData;

private:
	bool PendingSave;
	
	bool PendingLoad;

	bool bIsLoadedGame;

	bool bIsLoadedAutosave;

	UFUNCTION()
	void SaveDone(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	UFUNCTION()
	void LoadDone(const FString& SlotName, const int32 UserIndex, class USaveGame* LoadedGameData);

	/** Seperate function to fill the instance with required data as loading can be done synchronously or asynchronously. */
	UFUNCTION()
	void LoadFillData(class UThesisSaveGame* LoadedGameData);
};
