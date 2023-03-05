// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayableController.h"
#include "GameFramework/GameState.h"
#include "GameFramework/InputSettings.h"
#include "Components/InputComponent.h"
#include "PlayableCameraManager.h"
#include "ThesisGameHUD.h"

APlayableController::APlayableController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	TeamId = FGenericTeamId(0);

	PlayerCameraManagerClass = APlayableCameraManager::StaticClass();
}

void APlayableController::PawnPendingDestroy(APawn* MyPawn)
{
	FVector CameraLocation = MyPawn->GetActorLocation() + FVector(0, 0, 300.0f);
	FRotator CameraRotation(-90.0f, 0.0f, 0.0f);
	FindDeathCameraSpot(CameraLocation, CameraRotation);

	Super::PawnPendingDestroy(MyPawn);

	SetSpectatorCamera(CameraLocation, CameraRotation);
}

bool APlayableController::FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation)
{
	const FVector CurrentLocation = GetPawn()->GetActorLocation();
	FRotator CurrentDirection = GetControlRotation();
	CurrentDirection.Pitch = -45.0f;

	const float YawOffsets[] = {0.0f, -100.0f, 90.0f, -90.0f, 45.0f, -45.0f, 135.0f, -135.0f};
	const float DirectionOffset = 600.0f;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(DeathCamera), true, GetPawn());

	FHitResult HitResult;

	for (int32 i = 0; i < UE_ARRAY_COUNT(YawOffsets); i++)
	{
		FRotator NewDirection = CurrentDirection;
		NewDirection.Yaw += YawOffsets[i];
		NewDirection.Normalize();

		const FVector TestLocation = CurrentLocation - NewDirection.Vector() * DirectionOffset;

		const bool bBlockedView = GetWorld()->LineTraceSingleByChannel(HitResult, CurrentLocation, TestLocation, ECC_Camera, TraceParams);

		if (!bBlockedView)
		{
			CameraLocation = TestLocation;
			CameraRotation = NewDirection;
			return true;
		}
	}

	return false;
}

void APlayableController::SetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation)
{
	SetInitialLocationAndRotation(CameraLocation, CameraRotation);
	SetViewTarget(this);
}

FGenericTeamId APlayableController::GetGenericTeamId() const
{
	return TeamId;
}

void APlayableController::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	if (InTeamID != NULL)
	{
		TeamId = InTeamID;
	}
}

void APlayableController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// UI input
	InputComponent->BindAction("ViewLog", IE_Pressed, this, &APlayableController::ToggleDisplayLog).bExecuteWhenPaused = true;
}

void APlayableController::ToggleDisplayLog()
{
	if (GEngine->GameViewport == nullptr)
	{
		return;
	}

	AThesisGameHUD* PlayerHUD = GetHUD<AThesisGameHUD>();
	if (PlayerHUD != nullptr)
	{
		if (bIsViewingQuestLog)
		{
			PlayerHUD->DisableQuestLogWidget();
			bIsViewingQuestLog = false;
		}
		else
		{
			PlayerHUD->EnableQuestLogWidget();
			bIsViewingQuestLog = true;
		}
	}
}


//void APlayableController::BeginInactiveState()
//{
//	const AGameStateBase* GameState = GetWorld()->GetGameState();
//	const float MinRespawnDelay = GameState ? GameState->GetPlayerRespawnDelay(this) : 1.0f;
//	GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &APlayableController::Respawn, MinRespawnDelay);
//}
//
//void APlayableController::Respawn()
//{
//	UWorld* World = GetWorld();
//	if (World)
//	{
//		AGameModeBase* GameMode = World->GetAuthGameMode();
//		if (GameMode)
//		{
//			GameMode->RestartPlayer(this);
//		}
//	}
//}