// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayableCharacter.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Pickup.h"
#include "ThesisGameInstance.h"
#include "ThesisSaveGame.h"
#include "SaveGameStructs.h"
#include "PlayableController.h"
#include "PlayableState.h"
#include "BotAIController.h"
#include "Weapons/WeaponMaster.h"

// Sets default values
APlayableCharacter::APlayableCharacter()
	//: Super(ObjectInitializer.SetDefaultSubobjectClass<UShooterCharacterMovement>(ACharacter::CharacterMovementComponentName)
{
	// Set correct capsule size
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Set source for stimuli
	StimuliSourceComp = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComp"));
	StimuliSourceComp->bAutoRegister = true;

	// Create the camera component only if player
	ABotAIController* BotAI = Cast<ABotAIController>(Controller);
	if (BotAI == NULL)
	{
		// Init camera defaults
		//NormalFOV = 90.0f;
		//IronSightsFOV = 60.0f;
		//DefaultFOV = NormalFOV;

		//Camera1P = CreateDefaultSubobject<UCameraComponent>(TEXT("CharacterCamera1P"));
		//check(Camera1P != nullptr);

		//Attach it
		//Camera1P->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
		//Camera1P->SetRelativeLocation(FVector(-39.56f, 1.75f, 5.f + BaseEyeHeight));
		//Camera1P->bUsePawnControlRotation = true;
		//Camera1P->SetFieldOfView(NormalFOV);

		//APlayerCameraManager* MyCamManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 1);
		//if (MyCamManager)
		//{
		//	MyCamManager->ViewPitchMin = -87.0f;
		//	MyCamManager->ViewPitchMax = 87.0f;
		//}

		// Create a mesh component for the first person view
		Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
		//Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("CharacterMesh1P"));
		check(Mesh1P != nullptr);

		//Mesh1P->SetupAttachment(Camera1P);
		Mesh1P->SetupAttachment(GetCapsuleComponent());
		Mesh1P->bOnlyOwnerSee = true;
		Mesh1P->bOwnerNoSee = false;
		Mesh1P->bReceivesDecals = true;
		Mesh1P->bCastDynamicShadow = false;
		Mesh1P->CastShadow = false;
		Mesh1P->bReceivesDecals = false;
		Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh1P->SetCollisionObjectType(ECC_Pawn);
		Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
		//Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
		//Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

		//InteractionCollisionComp = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("InteractiveCollision"));
		InteractionCollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractiveCollision"));
		InteractionCollisionComp->SetBoxExtent(FVector(40.0f, 40.0f, 80.0f), true);
		InteractionCollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		InteractionCollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
		InteractionCollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		InteractionCollisionComp->SetCollisionResponseToChannel(COLLISION_PICKUP, ECR_Overlap);
		InteractionCollisionComp->CanCharacterStepUpOn = ECB_No;
		InteractionCollisionComp->SetupAttachment(GetCapsuleComponent());
		InteractionCollisionComp->SetRelativeLocation(FVector(70.0f, 0.0f, 0.0f));
		InteractionCollisionComp->MoveIgnoreActors.Add(GetInstigator());
	}

	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->bReceivesDecals = false;
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	//GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); //x = 5.2 y = 2.0 z = -90.0
	//GetMesh()->SetRelativeLocation(FVector(1.30f, -4.14f, -90.0f));

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);


 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default properties
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	bIsDying = false;
	bIsCrouching = false;
	bIsProning = false;
	bIsFiring = false;
	bIsTargeting = false;
	bIsSprinting = false;
	bIsViewingQuestLog = false;
	bCanPickup = false;

	//Variables pointing to speed ranges, for fine tuning
	fWalkingSpeed = 0.4f;
	fFiringSpeed = 0.3f;
	fTargetingSpeed = 0.25f;
	bPendingSpeedChange = false;

	//set default speed to walking
	fSpeedRatio = fWalkingSpeed;
}

void APlayableCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//Set a pointer to the game instance
	UWorld* World = GetWorld();
	if (World)
	{
		CurrentGameInstance = Cast<UThesisGameInstance>(UGameplayStatics::GetGameInstance(World));
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		// Load things from save or set default
		if (CurrentGameInstance != nullptr && CurrentGameInstance->IsLoadedGame())
		{
			GetWorldTimerManager().SetTimerForNextTick(this, &APlayableCharacter::LoadCharacterDefaults);
		}
		else
		{
			fCharacterBaseHealth = GetBaseHealth();
			fCharacterHealth = fCharacterBaseHealth;
			GetWorldTimerManager().SetTimerForNextTick(this, &APlayableCharacter::SpawnDefaultInventory);
		}
	}

	// set initial mesh visibility (3rd person view)
	UpdatePawnMeshes();

	StimuliSourceComp->RegisterForSense(UAISense_Sight::StaticClass());
	StimuliSourceComp->RegisterForSense(UAISense_Hearing::StaticClass());
	UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Sight::StaticClass(), this);
	UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Hearing::StaticClass(), this);

	check(InteractionCollisionComp != nullptr);
	InteractionCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &APlayableCharacter::OnInteractionCollisionOverlap);
	InteractionCollisionComp->OnComponentEndOverlap.AddDynamic(this, &APlayableCharacter::OnInteractionCollisionEndOverlap);
}

// Called when the game starts or when spawned
void APlayableCharacter::BeginPlay()
{
	Super::BeginPlay();
}

//Clean up character
void APlayableCharacter::BeginDestroy()
{
	Super::BeginDestroy();
}

void APlayableCharacter::Destroyed()
{
	Super::Destroyed();
	DestroyInventory();
}

void APlayableCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// switch mesh to 1st person view
	UpdatePawnMeshes();

	// reattach weapon if needed
	EquipWeapon(CurrentWeapon);
}

float APlayableCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (fCharacterHealth <= 0.0f)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		fCharacterHealth -= ActualDamage;
		if (fCharacterHealth <= 0.0f)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}

		MakeNoise(0.3f, EventInstigator ? EventInstigator->GetPawn() : this);
	}

	return ActualDamage;
}

bool APlayableCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	//|| GetWorld()->GetAuthGameMode<AThesisGameGameMode>() == NULL
	if (bIsDying || IsPendingKill() || GetLocalRole() != ROLE_Authority)
	{
		return false;
	}

	return true;
}

bool APlayableCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("Pawn trying to die."));
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		UE_LOG(LogTemp, Error, TEXT("Pawn cannot die in current state."));
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("Pawn is dying."));

	fCharacterHealth = FMath::Min(0.0f, fCharacterHealth);

	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	//GetWorld()->GetAuthGameMode<AThesisGameGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}

void APlayableCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	Die(fCharacterHealth, FDamageEvent(dmgType.GetClass()), NULL, NULL);
}

float APlayableCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* MyMesh = GetPawnMesh();
	if (MyMesh && MyMesh->AnimScriptInstance && AnimMontage)
	{
		return MyMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);
	}
	return 0.0f;
}

void APlayableCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* MyMesh = GetPawnMesh();
	if (MyMesh && MyMesh->AnimScriptInstance && AnimMontage && MyMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		MyMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOut.GetBlendTime(), AnimMontage);
	}
}

void APlayableCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* MyMesh = GetPawnMesh();
	if (MyMesh && MyMesh->AnimScriptInstance)
	{
		MyMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}

// Called every frame
void APlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Handle speed changes, one action at a time

	//When walking
	/*if (!bIsTargeting && !bIsSprinting && fSpeedRatio != fWalkingSpeed)
	{
		fSpeedRatio = FMath::Lerp(fSpeedRatio, fWalkingSpeed, 0.5f);
		if (IsFirstPerson() && DefaultFOV != NormalFOV)
		{
			DefaultFOV = FMath::FInterpTo(DefaultFOV, NormalFOV, DeltaTime, 20.0f);
			Camera1P->SetFieldOfView(DefaultFOV);
		}
	}*/

	//When targeting
	if (bIsTargeting)
	{
		if (fSpeedRatio > fTargetingSpeed)
		{
			fSpeedRatio = FMath::Lerp(fSpeedRatio, fTargetingSpeed, 0.25f);
		}
		/*if (IsFirstPerson() && DefaultFOV != IronSightsFOV)
		{
			DefaultFOV = FMath::FInterpTo(DefaultFOV, IronSightsFOV, DeltaTime, 20.0f);
			Camera1P->SetFieldOfView(DefaultFOV);
		}*/
	}

	//When sprinting
	if (bIsSprinting && fSpeedRatio < 1.0)
	{
		fSpeedRatio = FMath::Lerp(fSpeedRatio, 1.0f, 0.25f);
		/*if (IsFirstPerson() && DefaultFOV != NormalFOV)
		{
			DefaultFOV = FMath::FInterpTo(DefaultFOV, NormalFOV, DeltaTime, 20.0f);
			Camera1P->SetFieldOfView(DefaultFOV);
		}*/
	}

	//When firing
	if (bIsFiring && fSpeedRatio != fFiringSpeed)
	{
		fSpeedRatio = FMath::Lerp(fSpeedRatio, fFiringSpeed, 0.1f);
	}

	/*if (fCharacterHealth <= 0.0f)
	{
		Die(fCharacterHealth, FDamageEvent(UDamageType::StaticClass()), NULL, NULL);
	}*/
	if (GEngine->UseSound())
	{
		UpdateFootsteps();
	}
}

// Called to bind functionality to input
void APlayableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Set up jumping events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//Set up movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayableCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayableCharacter::MoveRight);

	//Set up looking events
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayableCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayableCharacter::LookUpAtRate);

	//Sprint
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayableCharacter::OnSprintStart);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayableCharacter::OnSprintStop);

	//Reload event
 	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayableCharacter::OnReload);

	//Set up firing events
	PlayerInputComponent->BindAction("FireWeapon", IE_Pressed, this, &APlayableCharacter::OnFireStart);
	PlayerInputComponent->BindAction("FireWeapon", IE_Released, this, &APlayableCharacter::OnFireStop);

	//Set up changing gear
	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &APlayableCharacter::OnNextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &APlayableCharacter::OnPrevWeapon);

	//Set up Iron Sights events
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &APlayableCharacter::OnIronSightsStart);
	PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &APlayableCharacter::OnIronSightsStop);

	//Melee attack
	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &APlayableCharacter::OnMelee);

	// Restart, testing purposes only
	PlayerInputComponent->BindAction("Restart", IE_Pressed, this, &APlayableCharacter::RestartPlayer);
}

//void APlayableCharacter::UpdateCamera(float DeltaTime)
//{
//	if (IsFirstPerson())
//	{
//		const float SelectedFOV = IsTargeting() ? IronSightsFOV : NormalFOV;
//		DefaultFOV = FMath::FInterpTo(DefaultFOV, SelectedFOV, DeltaTime, 20.0f);
//	}
//
//	Super::UpdateCamera(DeltaTime);
//}

//******************
// Implement input functionality

void APlayableCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
		Value *= fSpeedRatio;
		AddMovementInput(Direction, Value);
	}
}

void APlayableCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		FVector Direction = FQuatRotationMatrix(GetActorQuat()).GetScaledAxis(EAxis::Y);
		Value *= fSpeedRatio;
		AddMovementInput(Direction, Value);
	}
}

void APlayableCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayableCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayableCharacter::OnSprintStart()
{
	if (!bIsSprinting && !bIsTargeting && !bIsFiring)
	{
		bIsSprinting = true;
	}
}

void APlayableCharacter::OnSprintStop()
{
	if (bIsSprinting)
	{
		bIsSprinting = false;
	}
}

void APlayableCharacter::OnFireStart()
{
	if (!bIsFiring)
	{
		if (IsRunning())
		{
			bIsSprinting = false;
		}
		bIsFiring = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartFire();
			MakeNoise(1.0f, this, GetActorLocation());
		}
	}
}

void APlayableCharacter::OnFireStop()
{
	if (bIsFiring)
	{
		bIsFiring = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
	}
}

void APlayableCharacter::OnNextWeapon()
{
	APlayableController* MyPC = Cast<APlayableController>(Controller);
	if (MyPC && !MyPC->bCinematicMode)
	{
		if (CharacterInventory.Num() > 1 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponStates::Equipping))
		{
			const int32 CurrentWeaponIndex = CharacterInventory.IndexOfByKey(CurrentWeapon);
			AWeaponMaster* NextWeapon = CharacterInventory[(CurrentWeaponIndex + 1) % CharacterInventory.Num()];
			EquipWeapon(NextWeapon);
		}
	}
}

void APlayableCharacter::OnPrevWeapon()
{
	APlayableController* MyPC = Cast<APlayableController>(Controller);
	if (MyPC && !MyPC->bCinematicMode)
	{
		if (CharacterInventory.Num() > 1 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponStates::Equipping))
		{
			const int32 CurrentWeaponIndex = CharacterInventory.IndexOfByKey(CurrentWeapon);
			AWeaponMaster* PrevWeapon = CharacterInventory[(CurrentWeaponIndex - 1 + CharacterInventory.Num()) % CharacterInventory.Num()];
		}
	}
}

void APlayableCharacter::OnMelee()
{
	if (!bIsFiring)
	{
		bIsFiring = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartMelee();
		}
		bIsFiring = false;
	}
}

void APlayableCharacter::OnReload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
		MakeNoise(0.5f, this, GetActorLocation());
	}
}

void APlayableCharacter::OnIronSightsStart()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!bIsTargeting && !PC->bCinematicMode)
	{
		bIsTargeting = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartIronSights();
		}
	}

	if (TargetingSound)
	{
		UGameplayStatics::SpawnSoundAttached(TargetingSound, GetRootComponent());
	}
}

void APlayableCharacter::OnIronSightsStop()
{
	if (bIsTargeting)
	{
		bIsTargeting = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopIronSights();
		}
	}
}

void APlayableCharacter::RestartPlayer()
{
	//Get a reference to the Pawn Controller.
	AController* ControllerRef = GetController();

	//Destroy the Player.   
	Destroy();

	//Get the World and GameMode in the world to invoke its restart player function.
	UWorld* World = GetWorld();
	if (World)
	{
		AThesisGameGameMode* GameMode = World->GetAuthGameMode<AThesisGameGameMode>();
		if (GameMode)
		{
			GameMode->RestartPlayer(ControllerRef);
		}
	}
}

void APlayableCharacter::OnDeath_Implementation(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDeath triggered."));
	if (bIsDying)
	{
		return;
	}

	// Find if is player early
	LateControllerReference = Cast<APlayerController>(Controller);
	// Get reference to game mode and broadcast that player died
	// Ensure only a player gets access to this code, else we'll respawn each time we kill an enemy!
	AThesisGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
	if (GameMode)
	{
		GameMode->GetOnPlayerDeath().Broadcast(this);
	}

	TearOff();
	bIsDying = true;

	if (DeathSound && Mesh1P && Mesh1P->IsVisible())
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	//Remove weapons
	DestroyInventory();

	//switch to third person
	UpdatePawnMeshes();

	DetachFromControllerPendingDestroy();
	StopAllAnimMontages();

	if (RunLoopAC)
	{
		RunLoopAC->Stop();
	}

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);

	// Death anim
	float DeathAnimDuration = PlayAnimMontage(DeathAnim);

	// Ragdoll
	if (DeathAnimDuration > 0.0f)
	{
		// Trigger ragdoll before the animation stops
		const float TriggerRagdollTime = DeathAnimDuration - 0.4f;

		// Enable blend physics
		GetMesh()->bBlendPhysics = true;

		// Use a local timer handle
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &APlayableCharacter::SetRagdollPhysics, FMath::Max(0.1f, TriggerRagdollTime), false);
	}
	else
	{
		SetRagdollPhysics();
	}

	// Disable collisions on capsule, let skeleton ragdoll
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void APlayableCharacter::OnInteractionCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != nullptr)
	{
		
		// then construct the widget
		//AThesisGameGameMode* MyGameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
		//if (MyGameMode != nullptr)
		//{
		//	AThesisGameHUD* PlayerHUD = MyGameMode->GetPlayerHUD();
		//	if (PlayerHUD)
		//	{
		//		// First describe what the player is overlapping with
		//		PlayerHUD->DetermineInteraction(nullptr, OtherActor);
		//		// Then enable the widget that is filled with the interaction string
		//		PlayerHUD->EnableInteractionWidget(OtherComp);
		//	}
		//}
		
		if (OtherActor != nullptr)
		{
			APickup* MyPickup = Cast<APickup>(OtherActor);
			if (MyPickup != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Start overlap."));

				bCanPickup = true;
				APlayerController* PC = Cast<APlayerController>(Controller);
				if (PC)
				{
					AThesisGameHUD* PlayerHUD = Cast<AThesisGameHUD>(PC->GetHUD());
					if (PlayerHUD)
					{
						// First describe what the player is overlapping with
						PlayerHUD->DetermineInteraction(nullptr, OtherActor);
						// Then enable the widget that is filled with the interaction string
						PlayerHUD->EnableInteractionWidget(OtherActor->GetRootComponent());
					}
				}
				MyPickup->OnPickupBeginOverlap(this);
			}
		}
	}
}

void APlayableCharacter::OnInteractionCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("End overlap."));
	
	// disable the widget
	/*AThesisGameGameMode* MyGameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
	if (MyGameMode != nullptr)
	{
		AThesisGameHUD* PlayerHUD = MyGameMode->GetPlayerHUD();
		if (PlayerHUD)
		{
			PlayerHUD->DisableInteractionWidget();
		}
	}*/
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC)
	{
		AThesisGameHUD* PlayerHUD = Cast<AThesisGameHUD>(PC->GetHUD());
		if (PlayerHUD)
		{
			PlayerHUD->DisableInteractionWidget();
		}
	}

	bCanPickup = false;
	APickup* MyPickup = Cast<APickup>(OverlappedComponent);
	if (MyPickup)
	{
		MyPickup->OnPickupEndOverlap(this);
	}
}

//*******************
//Implement inventory
void APlayableCharacter::SpawnDefaultInventory()
{
	int32 NumWeaponClasses = this->DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (this->DefaultInventoryClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AWeaponMaster* NewWeapon = GetWorld()->SpawnActor<AWeaponMaster>(DefaultInventoryClasses[i], SpawnInfo);
			
			// Set infinite property for Bot pawns
			ABotAIController* BotController = Cast<ABotAIController>(Controller);
			if (BotController)
			{
				NewWeapon->SetInfiniteAmmo(true);
			}

			AddWeapon(NewWeapon);
		}
	}

	//If inventory is filled, equip first weapon
	if (CharacterInventory.Num() > 0)
	{
		EquipWeapon(CharacterInventory[0]);
	}
}

void APlayableCharacter::SpawnLoadedInventory(const FActorData& ActorData)
{
	// Load inventory
	for (const FInventoryData& Data : ActorData.LastInventory)
	{
		if (Data.WeaponClass)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AWeaponMaster* NewWeapon = GetWorld()->SpawnActor<AWeaponMaster>(Data.WeaponClass, SpawnInfo);
			if (NewWeapon)
			{
				NewWeapon->LoadWeaponData(Data);
				AddWeapon(NewWeapon);
			}
		}
	}

	//If inventory is filled, equip last weapon or just the first
	if (CharacterInventory.Num() > 0)
	{
		const FInventoryData& EquippedWeaponData = ActorData.LastEquippedWeapon;
		if (EquippedWeaponData.WeaponClass)
		{
			UE_LOG(LogTemp, Display, TEXT("Equipping loaded weapon (%s)"), *EquippedWeaponData.WeaponClass->GetName());
			AWeaponMaster* LastWeapon = FindWeapon(EquippedWeaponData.WeaponClass);
			if (LastWeapon)
			{
				EquipWeapon(LastWeapon);
			}
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Equipping first loaded weapon."));
			EquipWeapon(CharacterInventory[0]);
		}
	}
}

void APlayableCharacter::DestroyInventory()
{
	for (int32 i = 0; i < CharacterInventory.Num(); i++)
	{
		AWeaponMaster* Weapon = CharacterInventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon);
			Weapon->Destroy();
		}
	}
}

void APlayableCharacter::AddWeapon(AWeaponMaster* Weapon)
{
	if (Weapon)
	{
		Weapon->OnEnterInventory(this);
		CharacterInventory.AddUnique(Weapon);
	}
}

void APlayableCharacter::RemoveWeapon(AWeaponMaster* Weapon)
{
	if (Weapon)
	{
		Weapon->OnLeaveInventory();
		CharacterInventory.RemoveSingle(Weapon);
	}
}

AWeaponMaster* APlayableCharacter::FindWeapon(TSubclassOf<AWeaponMaster> WeaponClass)
{
	for (int32 i = 0; i < CharacterInventory.Num(); i++)
	{
		if (CharacterInventory[i] && CharacterInventory[i]->IsA(WeaponClass))
		{
			return CharacterInventory[i];
		}
	}

	return NULL;
}

void APlayableCharacter::EquipWeapon(AWeaponMaster* NewWeapon)
{
	AWeaponMaster* LastWeapon = nullptr;
	if (NewWeapon)
	{
		if (this->CurrentWeapon != NULL && NewWeapon != CurrentWeapon)
		{
			LastWeapon = this->CurrentWeapon;
			//unequip
			this->CurrentWeapon->OnUnEquip();
		}
		
		this->CurrentWeapon = NewWeapon;
		NewWeapon->SetOwningPawn(this);
		NewWeapon->OnEquip(LastWeapon);
	}
}

//*******************
//Startup updates
void APlayableCharacter::LoadCharacterDefaults()
{
	ABotAIController* BotAI = Cast<ABotAIController>(Controller);
	if (BotAI == NULL)
	{
		UThesisSaveGame* GameData = CurrentGameInstance->GetGameData();
		if (GameData)
		{
			const FActorData& PlayerActorData = GameData->PlayerData;
			UE_LOG(LogTemp, Display, TEXT("Getting info for player"));

			// Load player's health
			fCharacterHealth = PlayerActorData.CurrentHealth;
			fCharacterBaseHealth = GetBaseHealth();
			// Load player's inventory
			SpawnLoadedInventory(PlayerActorData);
			// If loading somehow failed, spawn defaults
			if (CharacterInventory.Num() <= 0)
			{
				SpawnDefaultInventory();
			}
		}
		else
		{
			SpawnDefaultInventory();
		}
	}
}

void APlayableCharacter::UpdateFootsteps()
{
	const bool bFootstepsPlaying = RunLoopAC != nullptr && RunLoopAC->IsActive();
	//const bool b = bIsSprinting || bIsMoving;

	// Don't bother playing the sounds unless we're running and moving.
	if (!bFootstepsPlaying && IsMoving())
	{
		if (RunLoopAC != nullptr)
		{
			RunLoopAC->Play();
		}
		else if (RunLoopSound != nullptr)
		{
			RunLoopAC = UGameplayStatics::SpawnSoundAttached(RunLoopSound, GetRootComponent());
			if (RunLoopAC != nullptr)
			{
				RunLoopAC->bAutoDestroy = false;
			}
		}
	}
	else
	{
		if (bFootstepsPlaying)
		{
			RunLoopAC->Stop();

			if (RunStopSound != nullptr)
			{
				UGameplayStatics::SpawnSoundAttached(RunStopSound, GetRootComponent());
			}
		}
	}
}

void APlayableCharacter::UpdatePawnMeshes()
{
	bool const bFirstPerson = IsFirstPerson();

	Mesh1P->VisibilityBasedAnimTickOption = !bFirstPerson ? EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered : EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	Mesh1P->SetOwnerNoSee(!bFirstPerson);

	GetMesh()->VisibilityBasedAnimTickOption = bFirstPerson ? EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered : EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->SetOwnerNoSee(bFirstPerson);
}

void APlayableCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (GetLocalRole() == ROLE_Authority)
	{

		// Play force feedback effect on player controller
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			UDamageType* DamageType = Cast<UDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && HitForceFeedback && false)
			{
				FForceFeedbackParameters FFParams;
				FFParams.Tag = "Damage";
				PC->ClientPlayForceFeedback(HitForceFeedback, FFParams);
			}
		}
	}

	if (DamageTaken > 0.f)
	{
		ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
	}


}

void APlayableCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;

	if (GetMesh() && GetMesh()->GetPhysicsAsset() && !IsPendingKill())
	{
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		bInRagdoll = true;
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	//if no ragdoll, hide mesh and clean up fast
	float fLifespan = 1.0f;
	if (!bInRagdoll)
	{
		TurnOff();
		SetActorHiddenInGame(true);
	}
	else
	{
		fLifespan = 10.0f;
	}
	SetLifeSpan(fLifespan);
}

//*******************
//Implement getters

bool APlayableCharacter::IsTargeting() const
{
	return bIsTargeting;
}

bool APlayableCharacter::IsFiring() const
{
	return bIsFiring;
}

bool APlayableCharacter::IsSprinting() const
{
	return bIsSprinting;
}

bool APlayableCharacter::IsRunning() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return bIsSprinting && !GetVelocity().IsZero() && (GetVelocity().GetSafeNormal2D() | GetActorForwardVector()) > -0.1;
}

bool APlayableCharacter::IsMoving() const
{
	return FMath::Abs(GetLastMovementInputVector().Size()) > 0;
}

float APlayableCharacter::GetBaseHealth() const
{
	return GetClass()->GetDefaultObject<APlayableCharacter>()->fCharacterHealth;
}

float APlayableCharacter::GetHealth() const
{
	return this->fCharacterHealth;
}

float APlayableCharacter::GetHealthPercentage() const
{
	//return (this->GetBaseHealth() != 0) ? (this->fCharacterHealth /GetBaseHealth()) : 0;
	return (this->fCharacterBaseHealth > 0) ? (this->fCharacterHealth / this->fCharacterBaseHealth) : 0;
}

void APlayableCharacter::AddHealth(float Amount)
{
	float AddedHealth = FMath::Min(fCharacterBaseHealth, this->fCharacterHealth + Amount);
	this->fCharacterHealth = AddedHealth;
}

USkeletalMeshComponent* APlayableCharacter::GetPawnMesh() const
{
	return IsFirstPerson() ? Mesh1P : GetMesh();
}

TArray<class AWeaponMaster*> APlayableCharacter::GetCurrentInventory() const
{
	if (IsAlive())
	{
		return CharacterInventory;
	}
	return TArray<class AWeaponMaster*>();
}

class AWeaponMaster* APlayableCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

void APlayableCharacter::OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation)
{
	//USkeletalMeshComponent* FPMesh = GetMesh1P();
	USkeletalMeshComponent* FPMesh = Cast<USkeletalMeshComponent>(GetClass()->GetDefaultSubobjectByName(TEXT("CharacterMesh1P")));
	const FMatrix MeshLS = FRotationTranslationMatrix(FPMesh->GetRelativeRotation(), FPMesh->GetRelativeLocation());
	const FMatrix LocalToWorld = ActorToWorld().ToMatrixWithScale();

	const FRotator CameraPitch(CameraRotation.Pitch, 0.0f, 0.0f);
	const FRotator CameraYaw(0.0f, CameraRotation.Yaw, 0.0f);

	const FMatrix LeveledCameraLS = FRotationTranslationMatrix(CameraYaw, CameraLocation) * LocalToWorld.Inverse();
	const FMatrix PitchedCameraLS = FRotationMatrix(CameraPitch) * LeveledCameraLS;
	const FMatrix MeshRelativeToCamera = MeshLS * LeveledCameraLS.Inverse();
	const FMatrix PitchedMesh = MeshRelativeToCamera * PitchedCameraLS;

	Mesh1P->SetRelativeLocationAndRotation(PitchedMesh.GetOrigin(), PitchedMesh.Rotator());
}

FRotator APlayableCharacter::GetAimOffsets() const
{
	const FVector AimDirectionWS = GetBaseAimRotation().Vector();
	const FVector AimDirectionLS = ActorToWorld().InverseTransformVectorNoScale(AimDirectionWS);
	const FRotator AimRotatorLS = AimDirectionLS.Rotation();

	return AimRotatorLS;
}

bool APlayableCharacter::IsAlive() const
{
	return (this->fCharacterHealth > 0);
}

bool APlayableCharacter::IsFirstPerson() const
{
	return IsAlive() && Controller && Controller->IsLocalPlayerController();
}

bool APlayableCharacter::IsEnemyFor(AController* TestPC) const
{
	if (TestPC == Controller || TestPC == NULL)
	{
		return false;
	}

	if (Cast<ABotAIController>(Controller) != nullptr && Cast<APlayableController>(TestPC))
	{
		return true;
	}

	return false;

	bool bIsEnemy = true;
	if (GetWorld()->GetGameState())
	{
		const AThesisGameGameMode* LoadedGame = GetWorld()->GetGameState()->GetDefaultGameMode<AThesisGameGameMode>();
		if (LoadedGame)
		{

		}
	}
}

class AWeaponMaster* APlayableCharacter::GetWeapon() const
{
	return this->CurrentWeapon;
}

FName APlayableCharacter::GetWeaponSocket() const
{
	return this->WeaponSocket;
}

UAnimMontage* APlayableCharacter::GetEquipAnim() const
{
	return (IsFirstPerson()) ? EquipAnim.Anim1P : EquipAnim.Anim3P;
}

UAnimMontage* APlayableCharacter::GetReloadAnim() const
{
	return (IsFirstPerson()) ? ReloadAnim.Anim1P : ReloadAnim.Anim3P;
}

UAnimMontage* APlayableCharacter::GetFireAnim() const
{
	return (IsFirstPerson()) ? FireAnim.Anim1P : FireAnim.Anim3P;
}

bool APlayableCharacter::CanPickup() const
{
	return bCanPickup;
}

int32 APlayableCharacter::GetID() const
{
	return ActorID;
}

bool operator==(APlayableCharacter A, APlayableCharacter B)
{
	return A.ActorID == B.ActorID;
}