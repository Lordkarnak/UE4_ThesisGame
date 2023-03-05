// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ThesisGame.h"
#include "ThesisGameGameMode.h"
#include "GenericTeamAgentInterface.h"
#include "PlayableController.generated.h"

/**
 * 
 */
UCLASS()
class THESISGAME_API APlayableController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_UCLASS_BODY()
	
public:
	//APlayableController();

	virtual void PawnPendingDestroy(APawn* MyPawn) override;

	void SetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);

	/** sets up input */
	virtual void SetupInputComponent() override;

	/** what quest log to display */
	void ToggleDisplayLog();

	/*virtual void BeginInactiveState() override;

	void Respawn();*/

	/** IGenericTeamAgentInterface - Set the team ID for player */
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;

	/** IGenericTeamAgentInterface - Get the team ID for player */
	virtual FGenericTeamId GetGenericTeamId() const override;

protected:
	bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

	bool bIsViewingQuestLog;

private:
	FTimerHandle TimerHandle_Respawn;
	FGenericTeamId TeamId;
};
