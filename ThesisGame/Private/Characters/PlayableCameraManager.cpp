// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayableCameraManager.h"
#include "PlayableCharacter.h"

APlayableCameraManager::APlayableCameraManager(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	NormalFOV = 90.0f;
	IronSightsFOV = 60.0f;
	DefaultFOV = NormalFOV;
	ViewPitchMin = -87.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}

void APlayableCameraManager::UpdateCamera(float DeltaTime)
{
	APlayableCharacter* Player = PCOwner ? Cast<APlayableCharacter>(PCOwner->GetPawn()) : NULL;
	if (Player && Player->IsFirstPerson())
	{
		const float TargetFOV = Player->IsTargeting() ? IronSightsFOV : NormalFOV;
		DefaultFOV = FMath::FInterpTo(DefaultFOV, TargetFOV, DeltaTime, 20.0f);
	}

	Super::UpdateCamera(DeltaTime);

	if (Player && Player->IsFirstPerson())
	{
		Player->OnCameraUpdate(GetCameraLocation(), GetCameraRotation());
	}
}