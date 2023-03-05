// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CollidingPawn.generated.h"

UCLASS()
class THESISGAME_API ACollidingPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACollidingPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FVector2D MovementInput;
	FVector2D CameraInput;
	float ZoomFactor;
	float bZoomingIn;

	UPROPERTY()
		class UParticleSystemComponent* ParticleSystem;

	UPROPERTY()
		class UCollidingPawnMovementComponent* MovementComponent;

	UPROPERTY()
		class UCameraComponent* CameraComponent;

	UPROPERTY()
		class USpringArmComponent* SpringArmComponent;

	UPROPERTY()
		class UStaticMeshComponent* SphereVisual;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual UPawnMovementComponent* GetMovementComponent() const override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);
	void ParticleToggle();
	void ZoomIn();
	void ZoomOut();
};
