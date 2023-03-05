// Fill out your copyright notice in the Description page of Project Settings.


#include "CollidingPawn.h"
#include "CollidingPawnMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"

// Sets default values
ACollidingPawn::ACollidingPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Use sphere as root component that reacts to physics
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(40.0f);
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));

	//Create and position a 3D sphere
	SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	SphereVisual->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	if (SphereVisualAsset.Succeeded())
	{
		SphereVisual->SetStaticMesh(SphereVisualAsset.Object);
		SphereVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
		SphereVisual->SetWorldScale3D(FVector(0.8f));
	}

	//Create a particle system that we can activate and deactivate
	ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MovementParticles"));
	ParticleSystem->SetupAttachment(SphereVisual);
	ParticleSystem->bAutoActivate = false;
	ParticleSystem->SetRelativeLocation(FVector(-20.0f, 0.0f, 20.0f));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("/Game/StarterContent/Particles/P_Sparks.P_Sparks"));
	if (ParticleAsset.Succeeded())
	{
		ParticleSystem->SetTemplate(ParticleAsset.Object);
	}

	//Use a spring arm component to give the camera a smooth, natural feel
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraAttachmentArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-20.0f, 0.0f, 0.0f));
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = 3.0f;

	//Create a camera and attach it to the spring arm
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);

	//Take control of the pawn
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	MovementComponent = CreateDefaultSubobject<UCollidingPawnMovementComponent>(TEXT("CustomMovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;
}

// Called when the game starts or when spawned
void ACollidingPawn::BeginPlay()
{
	Super::BeginPlay();
	
	check(GEngine != nullptr);

	//Display a debug message
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Using CollidingPawn."));
}

// Called every frame
void ACollidingPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bZoomingIn)
	{
		ZoomFactor += DeltaTime / 0.5f; //Zoom in half a second
	}
	else
	{
		ZoomFactor -= DeltaTime / -0.25f; //Zoom out over a quarter a second
	}
	ZoomFactor = FMath::Clamp<float>(ZoomFactor, 0.0f, 1.0f);

	//Blend Camera FOV and SpringArm
	//NOT WORKING
	//CameraComponent->FieldOfView = FMath::Lerp<float>(90.0f, 60.0f, ZoomFactor);
	//SpringArmComponent->TargetArmLength = FMath::Lerp<float>(400.0f, 300.f, ZoomFactor);

	//Rotate our actor's yaw, which will turn the camera it's attachet to
	/*{
		FRotator NewRotation = GetActorRotation();
		NewRotation.Yaw += CameraInput.X;
		SetActorRotation(NewRotation);
	}*/

	//Rotate our camera's pitch and limit it
	/*{
		FRotator NewRotation = SpringArmComponent->GetComponentRotation();
		NewRotation.Pitch = FMath::Clamp<float>(NewRotation.Pitch + CameraInput.Y, -80.0f, -15.0f);
		SpringArmComponent->SetWorldRotation(NewRotation);
	}*/

	//Handle movement FAILED
	//{
	//	if (!MovementInput.IsZero())
	//	{
	//		MovementInput = MovementInput.GetSafeNormal() * 100;
	//		FVector NewLocation = GetActorLocation();
	//		NewLocation += GetActorForwardVector() * MovementInput.X * DeltaTime;
	//		NewLocation += GetActorRightVector() * MovementInput.Y * DeltaTime;
	//		//SetActorLocation(NewLocation);
	//		if (MovementComponent && (MovementComponent->UpdatedComponent == RootComponent))
	//		{
	//			MovementComponent->AddInputVector(NewLocation);
	//		}
	//	}
	//}
}

// Called to bind functionality to input
void ACollidingPawn::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);
	InInputComponent->BindAction("ParticleToggle", IE_Pressed, this, &ACollidingPawn::ParticleToggle);

	//Camera zoom
	InInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ACollidingPawn::ZoomIn);
	InInputComponent->BindAction("ZoomIn", IE_Released, this, &ACollidingPawn::ZoomOut);

	//Pawn movement
	InInputComponent->BindAxis("MoveForward", this, &ACollidingPawn::MoveForward);
	InInputComponent->BindAxis("MoveRight", this, &ACollidingPawn::MoveRight);

	//Camera movement
	InInputComponent->BindAxis("Turn", this, &ACollidingPawn::AddControllerYawInput);
	InInputComponent->BindAxis("LookUp", this, &ACollidingPawn::AddControllerPitchInput);
}

UPawnMovementComponent* ACollidingPawn::GetMovementComponent() const
{
	return MovementComponent;
}

//Input functions
void ACollidingPawn::MoveForward(float AxisValue)
{
	//TEST 1 working
	/*if (MovementComponent && (MovementComponent->UpdatedComponent == RootComponent))
	{
		MovementComponent->AddInputVector(GetActorForwardVector() * AxisValue);
	}*/
	//MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);

	//TEST 2
	//Get forward axis and move player to that direction
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, AxisValue);
}

void ACollidingPawn::MoveRight(float AxisValue)
{
	//TEST 1 working
	/*if (MovementComponent && (MovementComponent->UpdatedComponent == RootComponent))
	{
		MovementComponent->AddInputVector(GetActorRightVector() * AxisValue);
	}*/
	//MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);

	//TEST 2
	//Get right axis and move player to that direction
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, AxisValue);
}

void ACollidingPawn::Turn(float AxisValue)
{
	/*FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += AxisValue;
	SetActorRotation(NewRotation);*/
	CameraInput.X = AxisValue;
}

void ACollidingPawn::LookUp(float AxisValue)
{
	//FRotator NewRotation = SpringArm->getComponentRotation();
	//NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + RateValue, -80.f, -15.0f);
	//SpringArm->SetWorldRotation(NewRotation);
	CameraInput.Y = AxisValue;
}

void ACollidingPawn::ParticleToggle()
{
	if (ParticleSystem && ParticleSystem->Template)
	{
		ParticleSystem->ToggleActive();
	}
}

void ACollidingPawn::ZoomIn()
{
	bZoomingIn = true;
}

void ACollidingPawn::ZoomOut()
{
	bZoomingIn = false;
}