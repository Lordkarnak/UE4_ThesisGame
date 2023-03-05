// Fill out your copyright notice in the Description page of Project Settings.


#include "BotCharacter.h"
#include "BotAIController.h"
#include "ThesisGameInstance.h"
#include "SaveGameStructs.h"
#include "ThesisSaveGame.h"

ABotCharacter::ABotCharacter() : APlayableCharacter()
{
	AIControllerClass = ABotAIController::StaticClass();

	UpdatePawnMeshes();

	bUseControllerRotationYaw = true;

	PrimaryActorTick.bCanEverTick = true;

	//PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
}

void ABotCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	////Set a pointer to the game instance
	//UWorld* World = GetWorld();
	//if (World)
	//{
	//	CurrentGameInstance = Cast<UThesisGameInstance>(UGameplayStatics::GetGameInstance(World));
	//}
	//// Load things from save or set default
	//if (CurrentGameInstance != nullptr && CurrentGameInstance->IsLoadedGame())
	//{
	//	GetWorldTimerManager().SetTimerForNextTick(this, &ABotCharacter::LoadCharacterDefaults);
	//}
	//Super::PostInitializeComponents();
	/*if (PawnSensingComp)
	{
		PawnSensingComp->bHearNoises = true;
		PawnSensingComp->bOnlySensePlayers = false;
		PawnSensingComp->OnHearNoise.AddDynamic(this, &ABotCharacter::OnHearNoise);
	}*/
}

bool ABotCharacter::IsFirstPerson() const
{
	//Always false for bot characters
	return false;
}

void ABotCharacter::FaceRotation(FRotator NewRotation, float DeltaTime)
{
	FRotator CurrentRotation = FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 8.0f);

	Super::FaceRotation(CurrentRotation, DeltaTime);
}

void ABotCharacter::StartShooting()
{
	OnFireStart();
}

void ABotCharacter::StopShooting()
{
	OnFireStop();
}

void ABotCharacter::LoadCharacterDefaults()
{
	UThesisSaveGame* GameData = CurrentGameInstance->GetGameData();
	if (GameData)
	{
		UE_LOG(LogTemp, Display, TEXT("Loading bot defaults..."));
		if (GameData->BotData.Contains(ActorID))
		{
			const FActorData& BotActorData = GameData->BotData[ActorID];
			if (BotActorData.CurrentHealth <= 0)
			{
				Die(0, FDamageEvent(), nullptr, nullptr);
			}
			else
			{
				UE_LOG(LogTemp, Display, TEXT("Getting info for actor %d"), BotActorData.ActorID);
				UE_LOG(LogTemp, Display, TEXT("Loaded bot (%d) health: %f."), ActorID, fCharacterHealth);
				// Set bot's health
				fCharacterHealth = BotActorData.CurrentHealth;
				// Set bot's inventory
				SpawnLoadedInventory(BotActorData);
				// If loading somehow failed, spawn defaults
				if (CharacterInventory.Num() <= 0)
				{
					SpawnDefaultInventory();
				}
				// Set bot's location
				SetActorTransform(BotActorData.LastPosition, false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}
	else
	{
		SpawnDefaultInventory();
	}
}

//void ABotCharacter::OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume)
//{
//	UE_LOG(LogTemp, Warning, TEXT("Heard a noise!"));
//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Heard an Actor"));
//	if (PawnInstigator != this)
//	{
//		ABotAIController* MyAICon = Cast<ABotAIController>(Controller);
//		if (MyAICon)
//		{
//			MyAICon->SetSensedTarget(PawnInstigator, Location);
//		}
//	}
//}