// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponMaster.h"
#include "WeaponImpact.h"
#include "Characters/BotAIController.h"
#include "Perception/PawnSensingComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SaveGameStructs.h"

// Sets default values
AWeaponMaster::AWeaponMaster(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh1P"));
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = Mesh1P;

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh3P->SetupAttachment(Mesh1P);

	PawnNoiseEmitterComp = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("PawnNoiseEmitter"));

	bPlayingFireAnim = false;
	bIsEquipped = false;
	bIsFiring = false;
	bPendingEquip = false;
	bPendingReload = false;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AWeaponMaster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CurrentState = EWeaponStates::Idle;

	CurrentAmmo = 0;
	CurrentMaxAmmo = 0;

	LastFireTime = 0.0f;
	CurrentFiringSpread = 0.0f;
	CurrentFirepower = WeaponConfig.Firepower;

	if (WeaponConfig.InitialAmmo > 0)
	{
		if (WeaponConfig.InitialAmmo > WeaponConfig.MaxAmmo)
		{
			WeaponConfig.InitialAmmo = WeaponConfig.MaxAmmo;
		}
		CurrentAmmo = FMath::Min(WeaponConfig.InitialAmmo, WeaponConfig.MaxAmmoInClip);
		CurrentMaxAmmo = WeaponConfig.InitialAmmo;
	}

	/*if (AIPerceptionStimuliSource)
	{
		AIPerceptionStimuliSource->RegisterForSense(UAISense_Hearing::StaticClass());
	}*/

	DetachMeshFromPawn();
}

void AWeaponMaster::Destroyed()
{
	Super::Destroyed();

	StopSimulatingWeaponFire();
}

// Called when the game starts or when spawned
void AWeaponMaster::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//******************
//Events

void AWeaponMaster::OnEquip(const AWeaponMaster* LastWeapon)
{
	AttachMeshToPawn();
	bPendingEquip = true;

	if (LastWeapon && MyPawn)
	{
		float Duration = MyPawn->PlayAnimMontage(MyPawn->GetEquipAnim());
		if (Duration <= 0.0f)
		{
			//failsafe
			Duration = WeaponConfig.fDefaultEquipDuration;
		}
		EquipStartedTime = GetWorld()->GetTimeSeconds();
		EquipDuration = Duration;

		GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &AWeaponMaster::OnEquipFinished, Duration, false);
	}
	else
	{
		OnEquipFinished();
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}

void AWeaponMaster::OnEquipFinished()
{
	AttachMeshToPawn();
	bIsEquipped = true;
	bPendingEquip = false;

	//Determine the state so that reload works
	DetermineWeaponState();

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		//reload empty clip
		if (CurrentAmmo <= 0 && CanReload())
		{
			StartReload();
		}
	}
}

void AWeaponMaster::OnUnEquip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	StopFire();

	if (bPendingReload)
	{
		StopWeaponAnimation(MyPawn->GetReloadAnim());
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_StopReload);
		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopWeaponAnimation(MyPawn->GetEquipAnim());
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	DetermineWeaponState();
}

void AWeaponMaster::OnEnterInventory(APlayableCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void AWeaponMaster::OnLeaveInventory()
{
	if (IsAttachedToPawn())
	{
		OnUnEquip();
		SetOwningPawn(NULL);
	}
}

void AWeaponMaster::OnBurstStarted()
{
	// start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	float CurrentFireTime = LastFireTime + WeaponConfig.fShotTimeout;
	if (LastFireTime > 0 && WeaponConfig.fShotTimeout > 0.0f && CurrentFireTime > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeaponMaster::HandleFiring, CurrentFireTime - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AWeaponMaster::OnBurstFinished()
{
	StopSimulatingWeaponFire();

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;

	// reset firing interval adjustment
	TimerIntervalAdjustment = 0.0f;

	CurrentFiringSpread = 0.0f;
}


//******************
//Utilities

int32 AWeaponMaster::AddAmmo(int Amount)
{
	const int32 MissingAmmo = FMath::Max(0, WeaponConfig.MaxAmmo - CurrentMaxAmmo);
	Amount = FMath::Min(Amount, MissingAmmo);
	CurrentMaxAmmo += Amount;

	//AI implement

	//start reload if clip is empty
	if (CanReload() && MyPawn && MyPawn->GetWeapon() == this)
	{
		StartReload();
	}
	return Amount; // So we know how many rounds we used
}

void AWeaponMaster::RemoveAmmo(int Amount)
{
	// Prevent ammo from reaching negative values
	CurrentAmmo = FMath::Max(0, CurrentAmmo - Amount);
	//AI Implement
}

void AWeaponMaster::SetInfiniteAmmo(bool bInfinite)
{
	this->WeaponConfig.HasInfiniteAmmo = bInfinite;
}

void AWeaponMaster::StartFire()
{
	if (!bIsFiring)
	{
		bIsFiring = true;
		DetermineWeaponState();
	}
}

void AWeaponMaster::StopFire()
{
	if (bIsFiring)
	{
		bIsFiring = false;
		DetermineWeaponState();
	}
}

void AWeaponMaster::StartReload()
{
	if (CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		float Duration = PlayWeaponAnimation(MyPawn->GetReloadAnim());
		if (Duration <= 0.0f)
		{
			Duration = WeaponConfig.fDefaultReloadDuration;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &AWeaponMaster::StopReload, Duration, false);

		float ReloadDuration = FMath::Max(0.1f, Duration - 0.1f);
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AWeaponMaster::Reload, ReloadDuration, false);

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}

void AWeaponMaster::Reload()
{
	int32 ClipDelta = FMath::Min((WeaponConfig.MaxAmmoInClip - CurrentAmmo), CurrentMaxAmmo);
	if (ClipDelta > 0)
	{
		CurrentAmmo += ClipDelta;
		CurrentMaxAmmo = FMath::Max(0, CurrentMaxAmmo - ClipDelta);
	}
}

void AWeaponMaster::StopReload()
{
	if (CurrentState == EWeaponStates::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(MyPawn->GetReloadAnim());
	}
}

void AWeaponMaster::StartIronSights()
{
	if (WeaponConfig.CanTarget)
	{
		if (TargetingSound)
		{
			PlayWeaponSound(TargetingSound);
		}
		// Modify damage output
		CurrentFirepower = WeaponConfig.Firepower * 1.5f;
	}
}

void AWeaponMaster::StopIronSights()
{
	// Reset damage output
	CurrentFirepower = WeaponConfig.Firepower;
}

void AWeaponMaster::StartMelee()
{

}

void AWeaponMaster::SetOwningPawn(APlayableCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		SetInstigator(NewOwner);
		MyPawn = NewOwner;
		SetOwner(NewOwner);
	}
}

void AWeaponMaster::LoadWeaponData(const FInventoryData& Data)
{
	CurrentAmmo = Data.CurrentAmmo;
	CurrentMaxAmmo = Data.MaxAmmo;
}

void AWeaponMaster::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	if (ShouldDealDamage(Impact.GetActor()))
	{
		DealDamage(Impact, ShootDir);
	}

	// play FX
	const FVector EndTrace = Origin + ShootDir * WeaponConfig.Reach;
	const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;

	SpawnTrailEffect(EndPoint);
	SpawnImpactEffects(Impact);
}

void AWeaponMaster::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = WeaponConfig.DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = CurrentFirepower;

	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);

	APlayableCharacter* TargetCharacter = Cast<APlayableCharacter>(Impact.GetActor());
	if (TargetCharacter)
	{
		float newHP = TargetCharacter->GetHealth();
		FString target = TargetCharacter->GetClass()->GetFName().ToString();
		UE_LOG(LogTemp, Display, TEXT("Delt damage to character %s. Health: %f"), *target, newHP);
	}
}

//*********************
//Effects events

void AWeaponMaster::SimulateWeaponFire()
{
	if (MuzzleFX)
	{
		USkeletalMeshComponent* WeapMesh = GetWeaponMesh();
		if (MuzzlePSC == NULL)
		{
			if (MyPawn != NULL && MyPawn->IsLocallyControlled())
			{
				AController* PC = MyPawn->Controller;
				if (PC != NULL)
				{
					Mesh1P->GetSocketLocation(MuzzleSocket);
					MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, MuzzleSocket);
					MuzzlePSC->bOwnerNoSee = false;
					MuzzlePSC->bOnlyOwnerSee = true;

					Mesh3P->GetSocketLocation(MuzzleSocket);
					MuzzlePSCSecondary = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, MuzzleSocket);
					MuzzlePSCSecondary->bOwnerNoSee = true;
					MuzzlePSCSecondary->bOnlyOwnerSee = false;
				}
			}
			else
			{
				MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, WeapMesh, MuzzleSocket);
			}
		}
	}

	if (!bPlayingFireAnim)
	{
		PlayWeaponAnimation(MyPawn->GetFireAnim());
		bPlayingFireAnim = true;
	}

	if (FireAC == NULL)
	{
		FireAC = PlayWeaponSound(FireLoopSound);
	}
	else
	{
		PlayWeaponSound(FireSound);
	}

	APlayerController* PC = (MyPawn != NULL) ? Cast<APlayerController>(MyPawn->Controller) : NULL;
	if (PC != NULL && PC->IsLocalController())
	{
		if (FireCameraShake != NULL)
		{
			PC->ClientPlayCameraShake(FireCameraShake, 1);
		}
		if (FireForceFeedback != NULL)
		{
			FForceFeedbackParameters FFParams;
			FFParams.Tag = "Weapon";
			PC->ClientPlayForceFeedback(FireForceFeedback, FFParams);
		}
	}
}

void AWeaponMaster::StopSimulatingWeaponFire()
{
	if (MuzzlePSC != NULL)
	{
		MuzzlePSC->DeactivateSystem();
		MuzzlePSC = NULL;
	}
	if (MuzzlePSCSecondary != NULL)
	{
		MuzzlePSCSecondary->DeactivateSystem();
		MuzzlePSCSecondary = NULL;
	}
	if (bPlayingFireAnim)
	{
		if (MyPawn)
		{
			MyPawn->StopAnimMontage(MyPawn->GetFireAnim());
		}
		bPlayingFireAnim = false;
	}
	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;
		PlayWeaponSound(FireFinishSound);
	}
}

void AWeaponMaster::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		FHitResult UseImpact = Impact;

		//trace to find component
		if (!Impact.Component.IsValid())
		{
			const FVector StartTrace = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
			const FVector EndTrace = Impact.ImpactPoint - Impact.ImpactNormal * 10.0f;
			FHitResult Hit = WeaponTrace(StartTrace, EndTrace);
			UseImpact = Hit;
		}

		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint);
		AWeaponImpact* EffectActor = GetWorld()->SpawnActorDeferred<AWeaponImpact>(ImpactTemplate, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = UseImpact;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}
	}
}

void AWeaponMaster::SpawnTrailEffect(const FVector& EndPoint)
{
	if (TrailFX)
	{
		const FVector Origin = GetMuzzleLocation();

		UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, Origin);
		if (TrailPSC)
		{
			TrailPSC->SetVectorParameter(TrailTargetParam, EndPoint);
		}
	}
}

//*****************
//boolean checks

bool AWeaponMaster::IsEquipped() const
{
	return bIsEquipped;
}

bool AWeaponMaster::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}

bool AWeaponMaster::CanFire() const
{
	bool bCanFire = MyPawn && MyPawn->IsAlive();
	bool bStateOkToFire = (CurrentState == EWeaponStates::Idle) || (CurrentState == EWeaponStates::Firing);
	return (bCanFire == true) && (bStateOkToFire == true) && (bPendingReload == false);
}

bool AWeaponMaster::CanReload() const
{
	bool bCanReload = MyPawn && MyPawn->IsAlive();
	bool bGotAmmo = (CurrentAmmo < WeaponConfig.MaxAmmoInClip) && (GetMaxAmmo() > 0);
	bool bStateOKToReload = (CurrentState == EWeaponStates::Idle) || (CurrentState == EWeaponStates::Firing);
	return (bCanReload == true) && (bGotAmmo == true) && (bStateOKToReload == true);
}

float AWeaponMaster::GetCurrentSpread() const
{
	float FinalSpread = WeaponConfig.FiringSpread + CurrentFiringSpread;
	if (MyPawn && MyPawn->IsTargeting())
	{
		FinalSpread *= WeaponConfig.IronSightsSpreadModifier;
	}
	/*ABotAIController* Bot = Cast<ABotAIController>(MyPawn->GetController());
	if (Bot)
	{
		FinalSpread *= 0.35f;
	}*/

	return FinalSpread;
}

bool AWeaponMaster::ShouldDealDamage(AActor* TestActor) const
{
	if (TestActor)
	{
		if (TestActor->GetLocalRole() == ROLE_Authority || TestActor->GetTearOff())
		{
			return true;
		}
	}

	return false;
}

float AWeaponMaster::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float AWeaponMaster::GetEquipDuration() const
{
	return EquipDuration;
}

EWeaponStates::Type AWeaponMaster::GetCurrentState() const
{
	return CurrentState;
}

void AWeaponMaster::SetWeaponState(EWeaponStates::Type NewState)
{
	const EWeaponStates::Type PrevState = CurrentState;
	if (PrevState == EWeaponStates::Firing && NewState != EWeaponStates::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponStates::Firing && NewState == EWeaponStates::Firing)
	{
		OnBurstStarted();
	}
}

void AWeaponMaster::DetermineWeaponState()
{
	EWeaponStates::Type NewState = EWeaponStates::Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload() == false)
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = EWeaponStates::Reloading;
			}
		}
		else if ((bPendingReload == false) && (bIsFiring == true) && (CanFire() == true))
		{
			NewState = EWeaponStates::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponStates::Equipping;
	}

	SetWeaponState(NewState);
}

//******************
//Getters

int32 AWeaponMaster::GetMaxAmmo() const
{
	return CurrentMaxAmmo;
}

int32 AWeaponMaster::GetAmmoInClip() const
{
	return WeaponConfig.MaxAmmoInClip;
}

int32 AWeaponMaster::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

APlayableCharacter* AWeaponMaster::GetPawnOwner() const
{
	return MyPawn;
}

USkeletalMeshComponent* AWeaponMaster::GetWeaponMesh() const
{
	return (MyPawn && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}

FVector AWeaponMaster::GetAdjustedAim() const
{
	APlayerController* const PC = GetInstigatorController<APlayerController>();
	FVector FinalAim = FVector::ZeroVector;
	//if player controller, use it for aim
	//if (PC != NULL)
	//{
	//	FVector CamLoc;
	//	FRotator CamRot;
	//	PC->GetPlayerViewPoint(CamLoc, CamRot);
	//	FinalAim = CamRot.Vector();
	//}
	//else if (GetInstigator())
	//{
	//	//try to simplify aiming process and get rid of player camera inaccuracy
	//	//if AI controller, use it for aim
	//	//needs casting to AI controller
	//	AController* AICon = MyPawn ? Cast<ABotAIController>(MyPawn->Controller) : NULL;
	//	if (AICon != NULL)
	//	{
	//		FinalAim = AICon->GetControlRotation().Vector();
	//	}
	//	else
	//	{
	//		FinalAim =  GetInstigator()->GetBaseAimRotation().Vector();
	//	}
	//}
	FinalAim = MyPawn->Controller->GetControlRotation().Vector();

	return FinalAim;
}

FVector AWeaponMaster::GetCameraAim() const
{
	APlayerController* const PC = GetInstigatorController<APlayerController>();
	FVector FinalAim = FVector::ZeroVector;

	if (PC)
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (GetInstigator())
	{
		FinalAim = GetInstigator()->GetBaseAimRotation().Vector();
	}

	return FinalAim;
}

FVector AWeaponMaster::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	APlayerController* PC = MyPawn ? Cast<APlayerController>(MyPawn->Controller) : NULL;

	//Needs casting to AI controller
	ABotAIController* AIPC = MyPawn ? Cast<ABotAIController>(MyPawn->Controller) : NULL;
	FVector OutStartTrace = FVector::ZeroVector;

	// use player's camera
	if (PC)
	{
		FRotator UnusedRot;
		PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * ((GetInstigator()->GetActorLocation() - OutStartTrace) | AimDir);
	}
	//else use NPC weapon socket
	else if (AIPC)
	{
		OutStartTrace = GetMuzzleLocation();
	}

	return OutStartTrace;
}

//*****************
//Public functions

void AWeaponMaster::FireWeapon()
{
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const FVector AimDir = GetAdjustedAim();
	//const FVector StartTrace = GetCameraDamageStartLocation(AimDir);
	const FVector StartTrace = GetMuzzleLocation();
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * WeaponConfig.Reach;

	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	ProcessInstantHit(Impact, StartTrace, ShootDir, RandomSeed, CurrentSpread);

	CurrentFiringSpread = FMath::Min(WeaponConfig.FiringSpreadLimit, CurrentFiringSpread + WeaponConfig.FiringSpreadModifier);
	//UE_LOG(LogTemp, Display, TEXT("Firing spread: %f"), CurrentSpread);
}

void AWeaponMaster::HandleRefiring()
{
	// Update TimerIntervalAdjustment
	UWorld* MyWorld = GetWorld();

	float SlackTimeThisFrame = FMath::Max(0.0f, (MyWorld->TimeSeconds - LastFireTime) - WeaponConfig.fShotTimeout);

	if (bAllowAutomaticWeaponCatchup)
	{
		TimerIntervalAdjustment -= SlackTimeThisFrame;
	}

	HandleFiring();
}

void AWeaponMaster::HandleFiring()
{
	if (CurrentAmmo > 0 && CanFire())
	{
		SimulateWeaponFire();

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();
			if (!WeaponConfig.HasInfiniteAmmo)
			{
				RemoveAmmo(1);
			}
		}
	}
	else if (CanReload())
	{
		StartReload();
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (CurrentAmmo == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
			APlayerController* MyPC = Cast<APlayerController>(MyPawn->Controller);
		}

		// stop weapon fire FX, but stay in Firing state
		OnBurstFinished();
	}
	else
	{
		OnBurstFinished();
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// reload after firing last round
		if (CurrentAmmo <= 0 && CanReload())
		{
			StartReload();
		}

		// setup refire timer
		bRefiring = (CurrentState == EWeaponStates::Firing && WeaponConfig.fShotTimeout > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeaponMaster::HandleRefiring, FMath::Max<float>(WeaponConfig.fShotTimeout + TimerIntervalAdjustment, SMALL_NUMBER), false);
			TimerIntervalAdjustment = 0.f;
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void AWeaponMaster::AttachMeshToPawn()
{
	if (MyPawn)
	{
		//Remove meshes
		DetachMeshFromPawn();

		//Find the weapon socket
		FName AttachPoint = MyPawn->GetWeaponSocket();
		
		if (MyPawn->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh1P = MyPawn->GetMesh1P();
			
			if (Mesh1P != nullptr && PawnMesh1P != nullptr && PawnMesh1P->DoesSocketExist(AttachPoint))
			{
				Mesh1P->SetHiddenInGame(!MyPawn->IsFirstPerson());
				Mesh1P->AttachToComponent(PawnMesh1P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			}

			USkeletalMeshComponent* PawnMesh3P = MyPawn->GetMesh();
			
			if (Mesh3P != nullptr && PawnMesh3P != nullptr && PawnMesh3P->DoesSocketExist(AttachPoint))
			{
				Mesh3P->SetHiddenInGame(MyPawn->IsFirstPerson());
				Mesh3P->AttachToComponent(PawnMesh3P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			}
		}
		else
		{
			USkeletalMeshComponent* WeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* PawnMesh = MyPawn->GetPawnMesh();
			if (MyPawn->IsFirstPerson())
			{
				Mesh3P->SetHiddenInGame(true);
			}
			else
			{
				Mesh1P->SetHiddenInGame(true);
			}
			
			
			if (WeaponMesh != nullptr && PawnMesh != nullptr && PawnMesh->DoesSocketExist(AttachPoint))
			{
				WeaponMesh->SetHiddenInGame(false);
				WeaponMesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			}
		}
	}
}

void AWeaponMaster::DetachMeshFromPawn()
{
	if (Mesh1P != nullptr)
	{
		Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		Mesh1P->SetHiddenInGame(true);
	}

	if (Mesh3P != nullptr)
	{
		Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		Mesh3P->SetHiddenInGame(true);
	}
}

FHitResult AWeaponMaster::WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const
{
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;
	if (bDebug)
	{
		DrawDebugLine(GetWorld(), TraceFrom, TraceTo, FColor::Green, false, 2.0f, 0, 1.0f);
	}

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceFrom, TraceTo, COLLISION_WEAPON, TraceParams);

	return Hit;
}

UAudioComponent* AWeaponMaster::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, MyPawn->GetRootComponent());
		if (PawnNoiseEmitterComp)
		{
			PawnNoiseEmitterComp->MakeNoise(this, 1.0f, GetActorLocation());
		}
		
	}

	return AC;
}

float AWeaponMaster::PlayWeaponAnimation(UAnimMontage* Animation)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		if (Animation)
		{
			Duration = MyPawn->PlayAnimMontage(Animation);
		}
	}

	return Duration;
}

void AWeaponMaster::StopWeaponAnimation(UAnimMontage* Animation)
{
	if (MyPawn)
	{
		if (Animation)
		{
			MyPawn->StopAnimMontage(Animation);
		}
	}
}

FVector AWeaponMaster::GetMuzzleLocation() const
{
	USkeletalMeshComponent* MyMesh = GetWeaponMesh();
	return MyMesh->GetSocketLocation(MuzzleSocket);
}

FVector AWeaponMaster::GetMuzzleDirection() const
{
	USkeletalMeshComponent* MyMesh = GetWeaponMesh();
	return MyMesh->GetSocketRotation(MuzzleSocket).Vector();
}