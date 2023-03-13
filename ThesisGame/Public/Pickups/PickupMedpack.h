// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "PickupMedpack.generated.h"

/**
 *
 */
UCLASS(Abstract)
class THESISGAME_API APickupMedpack : public APickup
{
	GENERATED_BODY()

public:
	//bool TransferPickup(APlayableCharacter* Pawn);
	virtual bool TransferPickup_Implementation(APlayableCharacter* Pawn) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = Pickup)
	float Health;
};
