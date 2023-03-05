// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveTerminal.h"
#include "Kismet/GameplayStatics.h"
#include "ThesisSaveGame.h"
#include "ThesisGameInstance.h"
#include "Characters/PlayableCharacter.h"

void ASaveTerminal::BeginPlay()
{
    Super::BeginPlay();
    
    // get a ref to game instance
    GameInstance = Cast<UThesisGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}
bool ASaveTerminal::TransferPickup(APlayableCharacter* Pawn)
{
    UE_LOG(LogTemp, Display, TEXT("Game instance class: %s"), *GameInstance->GetClass()->GetName());
	if (Pawn != NULL && GameInstance != NULL && !LinkedCheckpoint.IsNone())
	{
        if (!GameInstance->IsPendingSave())
        {
            if (GameInstance->SaveGame(this, SaveSlotName, SaveIndex))
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Terminal saved progress successfully!."));
            }
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Terminal is still saving progress. Check back later."));
        }
	}
    // Always return false to prevent the pickup from destroying our save point!
    return false;
}