// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponMaster.h"
#include "Pickups/Pickup.h"
#include "PickupAmmo.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class THESISGAME_API APickupAmmo : public APickup
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = Pickup)
	int32 Ammo;

	UPROPERTY(EditDefaultsOnly, Category = Pickup)
	TSubclassOf<AWeaponMaster> WeaponType;

	virtual bool TransferPickup(APlayableCharacter* Pawn) override; //class APlayableCharacter* Pawn

	UFUNCTION(BlueprintCallable)
	bool MatchesWeapon(UClass* WeaponClass);
};
