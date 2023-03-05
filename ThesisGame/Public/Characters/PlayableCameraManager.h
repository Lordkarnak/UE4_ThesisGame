// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "PlayableCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class THESISGAME_API APlayableCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()
	
public:

	/** The default FOV to choose if all else fail */
	float DefaultFOV;

	/** FOV used when walking/running */
	float NormalFOV;

	/** FOV used when using iron sights */
	float IronSightsFOV;

	virtual void UpdateCamera(float DeltaTime) override;
};
