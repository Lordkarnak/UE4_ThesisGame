// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupMedpack.h"
#include "Characters/BotAIController.h"

bool APickupMedpack::TransferPickup_Implementation(APlayableCharacter* Pawn)
{
	ABotAIController* BotAI = Cast<ABotAIController>(Pawn->GetController());
	if (BotAI == NULL)
	{
		Pawn->AddHealth(Health);
		const FString Msg = FString::Printf(TEXT("Picked up med pack."));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Msg);
		return true;
	}
	return false;
}

//bool APickup::TransferPickup_Implementation(APlayableCharacter* Pawn)
//{
//	return true;
//}