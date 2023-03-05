// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/PlayableCharacter.h"
#include "BotCharacter.generated.h"

//class UPawnSensingComponent;
/**
 * 
 */
UCLASS()
class THESISGAME_API ABotCharacter : public APlayableCharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* BotBehavior;

	bool IsFirstPerson() const override;

	void FaceRotation(FRotator NewRotation, float DeltaTime = 0.f) override;

	void PostInitializeComponents() override;

	ABotCharacter();

	virtual void StartShooting();

	virtual void StopShooting();

	void LoadCharacterDefaults() override;

	//UFUNCTION()
	//void OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume);

	/** Represents the AI sensing capabilities of this pawn */
	/*UPROPERTY(VisibleAnywhere, Category = "AI Stimuli")
	class UPawnSensingComponent* PawnSensingComp;*/
};
