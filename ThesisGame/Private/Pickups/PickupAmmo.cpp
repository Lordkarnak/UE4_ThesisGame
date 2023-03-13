// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupAmmo.h"

bool APickupAmmo::TransferPickup_Implementation(APlayableCharacter* Pawn)
{
	AWeaponMaster* MyWeapon = (Pawn ? Pawn->GetWeapon() : NULL);
	if (MyWeapon)
	{
		int32 AddedAmmo = MyWeapon->AddAmmo(Ammo);
		const FString Msg = FString::Printf(TEXT("Picked up %d rounds."), AddedAmmo);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Msg);
		return AddedAmmo > 0;
	}
	return false;
}

//bool APickup::TransferPickup_Implementation(APlayableCharacter* Pawn)
//{
//	return true;
//}

bool APickupAmmo::MatchesWeapon(UClass* WeaponClass)
{
	return WeaponType->IsChildOf(WeaponClass);
}