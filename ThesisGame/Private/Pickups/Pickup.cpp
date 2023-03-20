// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickup.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayableCharacter.h"

class APlayableCharacter;

// Sets default values
APickup::APickup(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CapsuleComponent = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("CapsuleComp"));
	CapsuleComponent->InitCapsuleSize(40.0f, 50.0f);
	CapsuleComponent->SetCollisionObjectType(COLLISION_PICKUP);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CapsuleComponent->SetGenerateOverlapEvents(true);
	RootComponent = CapsuleComponent;

	PickupPSC = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("PickupFX"));
	if (PickupPSC != nullptr)
	{
		PickupPSC->SetupAttachment(RootComponent);
	}
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void APickup::OnPickupBeginOverlap(class AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	OverlappedPawn = Cast<APlayableCharacter>(Other);
	if (OverlappedPawn && CanBePickedUp(OverlappedPawn))
	{
		APlayerController* OverlappedPC = Cast<APlayerController>(OverlappedPawn->GetController());
		EnableInput(OverlappedPC);
	}
}

void APickup::OnPickupEndOverlap(class AActor* Other)
{
	Super::NotifyActorEndOverlap(Other);
	
	OverlappedPawn = Cast<APlayableCharacter>(Other);
	if (OverlappedPawn)
	{
		APlayerController* MyPC = Cast<APlayerController>(OverlappedPawn->GetController());
		DisableInput(MyPC);
		OverlappedPawn = nullptr;
	}
}

bool APickup::CanBePickedUp(class APlayableCharacter* TestPawn) const
{
	return TestPawn->IsAlive() && TestPawn->CanPickup();
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
	BindToPlayerInput();
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::BindToPlayerInput()
{
	InputComponent = NewObject<UInputComponent>(this);
	InputComponent->RegisterComponent();
	if (InputComponent)
	{
		InputComponent->BindAction("Interact", IE_Pressed, this, &APickup::OnPickup);
	}
}

void APickup::OnPickup()
{
	if (OverlappedPawn && CanBePickedUp(OverlappedPawn) && !IsPendingKill())
	{
		UE_LOG(LogTemp, Display, TEXT("Pickup event fired."));
		if (TransferPickup(OverlappedPawn))
		{
			UGameplayStatics::SpawnSoundAttached(PickupSound, OverlappedPawn->GetRootComponent());
			DisableEffects();
			Destroy();
		}
	}
}

bool APickup::TransferPickup_Implementation(APlayableCharacter* Pawn)
{
	return true;
}

TEnumAsByte<EInteractionTypes::Type> APickup::GetPrompt() const
{
	return Prompt;
}

//bool APickup::TransferPickup_Implementation(APlayableCharacter* Pawn)
//{
//	return true;
//}

void APickup::DisableEffects()
{
	if (OverlappedPawn)
	{
		APlayerController* MyPC = Cast<APlayerController>(OverlappedPawn->GetController());
		DisableInput(MyPC);
	}
	OverlappedPawn = nullptr;
	PickupPSC->DeactivateSystem();
	InputComponent->Deactivate();
}