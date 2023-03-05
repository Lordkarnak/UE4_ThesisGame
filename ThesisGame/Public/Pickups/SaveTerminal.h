// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "SaveTerminal.generated.h"

/**
 * 
 */
UCLASS()
class THESISGAME_API ASaveTerminal : public APickup
{
	GENERATED_BODY()

	void BeginPlay() override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
	int32 SaveIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Save)
	FName LinkedCheckpoint;

protected:
	class UThesisGameInstance* GameInstance;

	virtual bool TransferPickup(APlayableCharacter* Pawn) override;

	bool PendingSave = false;
};
