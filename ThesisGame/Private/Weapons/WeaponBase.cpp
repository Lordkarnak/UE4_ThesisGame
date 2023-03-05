// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/WeaponBase.h"
#include "../ThesisGame.h"
#include "../ThesisGameCharacter.h"
#include "../ThesisGameHUD.h"
#include "Kismet/GameplayStatics.h"
#include "../ThesisGameProjectile.h"

// Sets default values
AWeaponBase::AWeaponBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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
	//Mesh3P->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	//Mesh3P->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	Mesh3P->SetupAttachment(Mesh1P);

	bLoopedMuzzleFX = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bIsFiring = false;
	bPendingReload = false;
	bPendingEquip = false;
	CurrentState = EWeaponState::Idle;

	CurrentAmmo = 0;
	CurrentAmmoInClip = 0;
	BurstCounter = 0;
	LastFireTime = 0.0f;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AWeaponBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (WeaponConfig.InitialClips > 0)
	{
		CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
		CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
	}

	if (Mesh1P == nullptr || Mesh3P == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("PostInitializeComponents: Mesh reference was found null. If attempting to call Attach/Detach operations, the engine will crash (again)."));
		return;
	}
	DetachMeshFromPawn();
}

void AWeaponBase::Destroyed()
{
	Super::Destroyed();

	//StopFire();
}

// Weapon usage
void AWeaponBase::OnEquip(const AWeaponBase* LastWeapon)
{
	if (Mesh1P == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("OnEquip: Mesh reference was found null. If attempting to call Attach/Detach operations, the engine will crash (again)."));
		return;
	}
	AttachMeshToPawn();

	bPendingEquip = true;
	CheckCurrentWeaponState();

	//Play an animation if weapon is valid
	if (LastWeapon)
	{
		float Duration = PlayWeaponAnimation(EquipAnim);
		if (Duration <= 0.0f)
		{
			Duration = 0.5f;
		}
		EquipStartedTime = GetWorld()->GetTimeSeconds();
		EquipDuration = Duration;

		UE_LOG(LogTemp, Error, TEXT("OnEquip: Duration of Equip anim -> %f"), Duration);
		GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &AWeaponBase::OnEquipFinished, Duration, false);
	}
	else
	{
		OnEquipFinished();
	}

	if (WeaponUser && WeaponUser->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}

	//TODO find a way to broadcast notifies
	//AThesisGameCharacter::NotifyEquipWeapon.Broadcast(WeaponUser, this);
}

void AWeaponBase::OnEquipFinished()
{
	if (Mesh1P == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("OnEquipFinished: Mesh reference was found null. If attempting to call Attach/Detach operations, the engine will crash (again)."));
		return;
	}
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	CheckCurrentWeaponState();

	if (WeaponUser)
	{
		//reload empty mag
		if (WeaponUser->IsLocallyControlled() && CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}
	}
}

void AWeaponBase::OnUnEquip()
{
	if (Mesh1P == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("OnUnEquip: Mesh reference was found null. If attempting to call Attach/Detach operations, the engine will crash (again)."));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("OnUnEquip: Equipping animation should be stopped."));
	DetachMeshFromPawn();

	bIsEquipped = false;
	StopFire();

	if (bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_StopReload);
		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	//TODO Find a way to broadcast notifies
	//AThesisGameCharacter::NotifyUnEquipWeapon.Broadcast(WeaponUser, this);

	CheckCurrentWeaponState();
}

void AWeaponBase::OnEnterInventory(AThesisGameCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void AWeaponBase::OnLeaveInventory()
{
	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}

	SetOwningPawn(NULL);
}

void AWeaponBase::AttachMeshToPawn()
{
	if (WeaponUser)
	{
		//Remove and hide 1P and 3P meshes
		if (Mesh1P == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("AttachMeshToPawn: Mesh reference was found null. If attempting to call Attach/Detach operations, the engine will crash (again)."));
			return;
		}
		DetachMeshFromPawn();

		FName AttachPoint = WeaponUser->GetWeaponAttachPoint();
		if (WeaponUser->IsLocallyControlled())
		{
			USkeletalMeshComponent* UserMesh1P = WeaponUser->GetMesh1P();
			Mesh1P->SetHiddenInGame(false);
			//Mesh3P->SetHiddenInGame(false);
			Mesh1P->AttachToComponent(UserMesh1P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			//Mesh3P->AttachToComponent(UserMesh3P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
	}
}

void AWeaponBase::DetachMeshFromPawn()
{
	if (Mesh1P == nullptr || Mesh3P == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("DetachMeshFromPawn: Mesh reference was found null. If attempting to call Attach/Detach operations, the engine will crash (again)."));
		return;
	}
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh1P->SetHiddenInGame(true);

	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->SetHiddenInGame(true);
}

void AWeaponBase::StartFire()
{
	if (!bIsFiring)
	{
		bIsFiring = true;
		CheckCurrentWeaponState();
	}
}

void AWeaponBase::StopFire()
{
	if (bIsFiring)
	{
		bIsFiring = false;
		CheckCurrentWeaponState();
	}
}

void AWeaponBase::StartReload()
{
	if (CanReload() || true)
	{
		bPendingReload = true;
		CheckCurrentWeaponState();

		UE_LOG(LogTemp, Error, TEXT("StartReload: play animation"));
		float AnimDuration = PlayWeaponAnimation(ReloadAnim);
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = WeaponConfig.NoAnimReloadDuration;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &AWeaponBase::StopReload, AnimDuration, false);
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AWeaponBase::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
		if (WeaponUser && WeaponUser->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}

void AWeaponBase::StopReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		CheckCurrentWeaponState();
		StopWeaponAnimation(ReloadAnim);
	}
}

// Control properties //
bool AWeaponBase::CanFire() const
{
	bool bCanFire = WeaponUser && WeaponUser->CanFire();
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanFire == true) && (bStateOKToFire == true) && (bPendingReload == false));
}

bool AWeaponBase::CanReload() const
{
	bool bCanReload = (!WeaponUser || WeaponUser->CanReload());
	bool bGotAmmo = (CurrentAmmo < WeaponConfig.AmmoPerClip) && (CurrentAmmo - CurrentAmmoInClip > 0);
	bool bStateCanReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanReload == true) && (bGotAmmo == true) && (bStateCanReload == true));
}

// Weapon usage //

void AWeaponBase::GiveAmmo(int Amount)
{
	const int32 RemainingAmmo = FMath::Max(0, WeaponConfig.MaxAmmo - CurrentAmmo);
	Amount = FMath::Min(Amount, RemainingAmmo);
	CurrentAmmo += Amount;

	//auto reload
	if (GetCurrentAmmoInClip() <= 0 && CanReload() && WeaponUser && WeaponUser->GetWeapon() == this)
	{
		StartReload();
	}
}

void AWeaponBase::UseAmmo()
{
	CurrentAmmoInClip--;
	CurrentAmmo--;

	//unfinished
}

void AWeaponBase::HandleRefiring()
{
	UWorld* MyWorld = GetWorld();

	float SlackTimeThisFrame = FMath::Max(0.0f, (MyWorld->TimeSeconds - LastFireTime) - WeaponConfig.TimeBetweenShots);

	if (bAllowAutomaticWeaponCatchup)
	{
		TimerIntervalAdjustment -= SlackTimeThisFrame;
	}

	HandleFiring();
}

void AWeaponBase::HandleFiring()
{
	if ((CurrentAmmoInClip > 0) && CanFire())
	{
		SimulateWeaponFire();

		if (WeaponUser)
		{
			FireWeapon();

			UseAmmo();

			BurstCounter++;
		}
	}
	else if (CanReload())
	{
		StartReload();
	}
	else if (WeaponUser)
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
			//AController* PlayerController = WeaponUser->GetController();
			//AThesisGameHUD* GameHUD = PlayerController ? Cast<AThesisGameHUD>(PlayerController->GetHUD()) : NULL;
			//if(GameHUD)
			//{

			//}
		}

		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}
	else
	{
		OnBurstFinished();
	}

	if (WeaponUser)
	{
		if (CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}

		//refire time
		bRefiring = (CurrentState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeaponBase::HandleRefiring, FMath::Max<float>(WeaponConfig.TimeBetweenShots + TimerIntervalAdjustment, SMALL_NUMBER), false);
			TimerIntervalAdjustment = 0.f;
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void AWeaponBase::FireWeapon()
{
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const FVector AimDir = GetAdjustedAim();
	const FVector StartTrace = GetCameraDamageStartLocation(AimDir);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * WeaponConfig.WeaponRange;

	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	ProcessInstantHit(Impact, StartTrace, ShootDir, RandomSeed, CurrentSpread);

	CurrentFiringSpread = FMath::Min(WeaponConfig.FiringSpreadMax, CurrentFiringSpread + WeaponConfig.FiringSpreadIncrement);
}

void AWeaponBase::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	// handle damage
	if (ShouldDealDamage(Impact.GetActor()))
	{
		DealDamage(Impact, ShootDir);
	}

	// play FX locally
	const FVector EndTrace = Origin + ShootDir * WeaponConfig.WeaponRange;
	const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;

	SpawnTrailEffect(EndPoint);
	SpawnImpactEffects(Impact);
}

bool AWeaponBase::ShouldDealDamage(AActor* TestActor) const
{
	// if we hit an NPC, deal damage
	// TODO cast to NPC!!!
	if (TestActor)
	{
		return true;
	}

	return false;
}

void AWeaponBase::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = WeaponConfig.DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = WeaponConfig.HitDamage;

	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, WeaponUser->Controller, this);
}

void AWeaponBase::ReloadWeapon()
{
	int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);

	if (ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
	}
}

void AWeaponBase::SpawnImpactEffects(const FHitResult& Impact)
{
	if (/*ImpactTemplate && */Impact.bBlockingHit)
	{
		FHitResult UseImpact = Impact;

		// trace again to find component lost during replication
		if (!Impact.Component.IsValid())
		{
			const FVector StartTrace = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
			const FVector EndTrace = Impact.ImpactPoint - Impact.ImpactNormal * 10.0f;
			FHitResult Hit = WeaponTrace(StartTrace, EndTrace);
			UseImpact = Hit;
		}

		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint);
		/*AShooterImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AShooterImpactEffect>(ImpactTemplate, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = UseImpact;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}*/
	}
}

void AWeaponBase::SpawnTrailEffect(const FVector& EndPoint)
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

void AWeaponBase::SetWeaponState(EWeaponState::Type NewState)
{
	const EWeaponState::Type PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void AWeaponBase::CheckCurrentWeaponState()
{
	EWeaponState::Type NewState = EWeaponState::Idle;

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
				NewState = EWeaponState::Reloading;
			}
		}
		else if ((bPendingReload == false) && (bIsFiring == true) && (CanFire() == true))
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}

void AWeaponBase::OnBurstStarted()
{
	//start firing, delay if needed
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f && (LastFireTime + WeaponConfig.TimeBetweenShots > GameTime))
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeaponBase::HandleFiring, LastFireTime + WeaponConfig.TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AWeaponBase::OnBurstFinished()
{
	// stop firing
	BurstCounter = 0;

	StopSimulatingWeaponFire();

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;

	TimerIntervalAdjustment = 0.0f;
	CurrentFiringSpread = 0.0f;
}

// Weapon usage helpers //

float AWeaponBase::GetCurrentSpread() const
{
	float FinalSpread = WeaponConfig.WeaponSpread + CurrentFiringSpread;
	/*if (WeaponUser && WeaponUser->IsTargeting())
	{
		FinalSpread *= InstantConfig.TargetingSpreadMod;
	}*/

	return FinalSpread;
}

UAudioComponent* AWeaponBase::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound && WeaponUser)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, WeaponUser->GetRootComponent());
	}

	return AC;
}

float AWeaponBase::PlayWeaponAnimation(const FWeaponAnim& Animation)
{
	float Duration = 0.0f;
	if (WeaponUser)
	{
		UAnimMontage* NewAnim = Animation.WeaponAnim1P;
		if (NewAnim)
		{
			Duration = WeaponUser->PlayAnimMontage(NewAnim);
		}
	}

	return Duration;
}

void AWeaponBase::StopWeaponAnimation(const FWeaponAnim& Animation)
{
	if (WeaponUser)
	{
		UAnimMontage* NewAnim = Animation.WeaponAnim1P;
		if (NewAnim)
		{
			WeaponUser->StopAnimMontage(NewAnim);
		}
	}
}

FVector AWeaponBase::GetCameraAim() const
{
	AController* const PC = WeaponUser->Controller;
	FVector FinalAim = FVector::ZeroVector;

	if (PC)
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}

	return FinalAim;
}

FVector AWeaponBase::GetAdjustedAim() const
{
	AController* const PC = WeaponUser ? WeaponUser->Controller : NULL;
	FVector FinalAim = FVector::ZeroVector;
	// If we have a player controller use it for the aim
	if (PC)
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}

	return FinalAim;
}

FVector AWeaponBase::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	AController* PC = WeaponUser ? WeaponUser->Controller : NULL;
	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		// use player's camera
		FRotator UnusedRot;
		PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * ((GetInstigator()->GetActorLocation() - OutStartTrace) | AimDir);
	}

	return OutStartTrace;
}

FVector AWeaponBase::GetMuzzleLocation() const
{
	USkeletalMeshComponent* MyMesh = GetWeaponMesh();
	return MyMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AWeaponBase::GetMuzzleDirection() const
{
	USkeletalMeshComponent* MyMesh = GetWeaponMesh();
	return MyMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

FHitResult AWeaponBase::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	return Hit;
}

void AWeaponBase::SetOwningPawn(AThesisGameCharacter* NewOwner)
{
	if (WeaponUser != NewOwner)
	{
		SetInstigator(NewOwner);
		WeaponUser = NewOwner;
		SetOwner(NewOwner);
	}
}

void AWeaponBase::SimulateWeaponFire()
{
	if (MuzzleFX)
	{
		USkeletalMeshComponent* WeapMesh = GetWeaponMesh();
		if (!bLoopedMuzzleFX || MuzzlePSC == NULL)
		{
			if (WeaponUser != NULL && WeaponUser->IsLocallyControlled())
			{
				AController* PC = WeaponUser->Controller;
				if (PC != NULL)
				{
					MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, MuzzleAttachPoint);
					MuzzlePSC->bOwnerNoSee = false;
					MuzzlePSC->bOnlyOwnerSee = true;

					/*Mesh3P->GetSocketLocation(MuzzleAttachPoint);
					MuzzlePSCSecondary = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, MuzzleAttachPoint);
					MuzzlePSCSecondary->bOwnerNoSee = true;
					MuzzlePSCSecondary->bOnlyOwnerSee = false;*/
				}
			}
			else
			{
				MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, WeapMesh, MuzzleAttachPoint);
			}
		}
	}

	if (!bLoopedFireAnim || !bPlayingFireAnim)
	{
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	if (bLoopedFireSound)
	{
		if (FireAC == NULL)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}

	APlayerController* PC = (WeaponUser != NULL) ? Cast<APlayerController>(WeaponUser->Controller) : NULL;
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

void AWeaponBase::StopSimulatingWeaponFire()
{
	if (bLoopedMuzzleFX)
	{
		if (MuzzlePSC != NULL)
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = NULL;
		}
		/*if (MuzzlePSCSecondary != NULL)
		{
			MuzzlePSCSecondary->DeactivateSystem();
			MuzzlePSCSecondary = NULL;
		}*/
	}

	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;

		PlayWeaponSound(FireFinishSound);
	}
}

USkeletalMeshComponent* AWeaponBase::GetWeaponMesh() const
{
	return (WeaponUser != NULL) ? Mesh1P : Mesh3P;
}

class AThesisGameCharacter* AWeaponBase::GetPawnOwner() const
{
	return WeaponUser;
}

bool AWeaponBase::IsEquipped() const
{
	return bIsEquipped;
}

bool AWeaponBase::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}

EWeaponState::Type AWeaponBase::GetCurrentState() const
{
	return CurrentState;
}

int32 AWeaponBase::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

int32 AWeaponBase::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}

int32 AWeaponBase::GetAmmoPerClip() const
{
	return WeaponConfig.AmmoPerClip;
}

int32 AWeaponBase::GetMaxAmmo() const
{
	return WeaponConfig.MaxAmmo;
}

float AWeaponBase::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float AWeaponBase::GetEquipDuration() const
{
	return EquipDuration;
}