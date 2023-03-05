// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "AudioThread.h"
#include "ThesisGame.h"
#include "ThesisGameGameMode.h"
#include "Pickup.generated.h"

class UCapsuleComponent;
class UParticleSystem;
class USoundCue;

UCLASS(abstract)
class THESISGAME_API APickup : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual void OnPickupBeginOverlap(class AActor* Other);

	virtual void OnPickupEndOverlap(class AActor* Other);

	virtual bool CanBePickedUp(class APlayableCharacter* TestPawn) const;

	void BindToPlayerInput();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void DisableEffects();

	void OnPickup();

	virtual bool TransferPickup(APlayableCharacter* Pawn);

protected:
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* ActiveFX;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	USoundCue* PickupSound;

	UPROPERTY(EditDefaultsOnly, Category=Collision)
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(Transient)
	APlayableCharacter* OverlappedPawn;

private:
	/** FX component */
	UPROPERTY(VisibleDefaultsOnly, Category = Effects)
	UParticleSystemComponent* PickupPSC;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
