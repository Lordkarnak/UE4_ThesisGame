// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class UAnimMontage;
class AThesisGameCharacter;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShake;
class UForceFeedbackEffect;
class USoundCue;

namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Equipping,
	};
}

USTRUCT()
struct FHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float ReticleSpread;

	UPROPERTY()
	int32 RandomSeed;
};

USTRUCT()
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	// Base Properties //
	/** Max ammo loaded */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxAmmo;

	/** How many ammo a clip holds */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 AmmoPerClip;

	/** Start the game with that many clips */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 InitialClips;

	/** Time between two consecutive shots */
	UPROPERTY(EditDefaultsOnly, Category = Internals)
	float TimeBetweenShots;

	/** Failsafe reload duration if weapon does not have an animation */
	UPROPERTY(EditDefaultsOnly, Category = Internals)
	float NoAnimReloadDuration;

	// Damage properties //

	/** base weapon fire spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float WeaponSpread;

	/** targeting spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float TargetingSpreadMod;

	/** continuous firing: spread increment */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float FiringSpreadIncrement;

	/** continuous firing: max increment */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float FiringSpreadMax;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float WeaponRange;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 HitDamage;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
	float AllowedViewDotHitDir;

	/** Default properties */
	FWeaponData()
	{
		MaxAmmo = 100;
		AmmoPerClip = 20;
		InitialClips = 4;
		TimeBetweenShots = 0.2f;
		NoAnimReloadDuration = 1.0f;

		WeaponSpread = 5.0f;
		TargetingSpreadMod = 0.25f;
		FiringSpreadIncrement = 1.0f;
		FiringSpreadMax = 10.0f;
		WeaponRange = 10000.0f;
		HitDamage = 10;
		DamageType = UDamageType::StaticClass();
		AllowedViewDotHitDir = 0.8f;
	}
};

USTRUCT()
struct FWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	/** 1st person anim played on this weapon */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* WeaponAnim1P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* WeaponAnim3P;
};

UCLASS(Abstract, Blueprintable)
class AWeaponBase : public AActor
{
	GENERATED_UCLASS_BODY()
	
	/** perform initial setup */
	virtual void PostInitializeComponents() override;

	virtual void Destroyed() override;

public:	
	// Sets default values for this actor's properties
	//AWeaponBase();
	
	// Ammo //

	/** Add ammo to weapon by taking clips */
	void GiveAmmo(int iAmount);

	/** Spend ammo by firing */
	void UseAmmo();

	// Usage //

	/** What happens when we equip/use the weapon */
	virtual void OnEquip(const AWeaponBase* LastWeapon);

	/** Weapon equipped */
	virtual void OnEquipFinished();

	/** What happens when we holster the weapon */
	virtual void OnUnEquip();

	/** Event fired when weapon is added to pawn's inventory */
	virtual void OnEnterInventory(AThesisGameCharacter* NewOwner);

	/** Event fired when weapon was removed from pawn's inventory */
	virtual void OnLeaveInventory();

	/** Determine if weapon is in use by a Pawn */
	bool IsEquipped() const;

	bool IsAttachedToPawn() const;

	// Input //

	/** Pawn starts firing the weapon */
	virtual void StartFire();

	/** Pawn ceases firing */
	virtual void StopFire();

	/** Pawn starts reloading the weapon */
	virtual void StartReload();

	/** Interrupt reloading functionality */
	virtual void StopReload();

	/** Finally, reload the weapon */
	virtual void ReloadWeapon();

	/** Query if weapon can be fired */
	bool CanFire() const;

	/** Query if weapon can reload */
	bool CanReload() const;

	// Getters //

	/** Get the state this weapon is in */
	EWeaponState::Type GetCurrentState() const;

	/** Get weapon's current ammo pool size */
	int32 GetCurrentAmmo() const;

	/** Get weapon's current ammo in clip */
	int32 GetCurrentAmmoInClip() const;

	/** Get weapon's clip size */
	int32 GetAmmoPerClip() const;

	/** Get weapon's max ammo capacity */
	int32 GetMaxAmmo() const;

	/** Get our mesh */
	USkeletalMeshComponent* GetWeaponMesh() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class AThesisGameCharacter* GetPawnOwner() const;

	/** Adjustment to handle frame rate affecting timer interval */
	UPROPERTY(Transient)
	float TimerIntervalAdjustment;

	/** Whether to allow automatic weapons to catch up with shorter refire cycles */
	UPROPERTY(Config)
	bool bAllowAutomaticWeaponCatchup = true;

	/** set the weapon owner */
	void SetOwningPawn(AThesisGameCharacter* NewOwner);

	/** Query the last time this weapon was switched to */
	float GetEquipStartedTime() const;

	/** Query the duration of equipping this weapon */
	float GetEquipDuration() const;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AThesisGameProjectile> ProjectileClass;

protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

	/** Who holds this weapon */
	UPROPERTY(Transient)
	class AThesisGameCharacter* WeaponUser;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FWeaponData WeaponConfig;

	/** Firing audio */
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	/** Name of muzzle bone/socket */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName MuzzleAttachPoint;

	/** Muzzle flash FX */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* MuzzleFX;

	/** Component for muzzle flash */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	/** Camera shake effect when firing */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<UCameraShake> FireCameraShake;

	/** Force feedback effect to play when firing */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UForceFeedbackEffect* FireForceFeedback;

	/** Single shot sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireSound;

	/** Looped shot sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireLoopSound;

	/** Finished burst sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireFinishSound;

	/** Out of ammo sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* OutOfAmmoSound;

	/** Reloading sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* ReloadSound;

	/** Equip sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* EquipSound;

	/** Muzzle FX looped? */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	uint32 bLoopedFireSound : 1;

	/** Is fire animation playing? */
	uint32 bPlayingFireAnim : 1;

	/** Is weapon equipped? */
	uint32 bIsEquipped : 1;

	/** Is weapon firing? */
	uint32 bIsFiring : 1;

	/** Is reloading animation playing */
	UPROPERTY(Transient)
	uint32 bPendingReload : 1;

	/** Is equip animation playing */
	uint32 bPendingEquip : 1;

	/** weapon is refiring */
	uint32 bRefiring;

	/** Current weapon state*/
	EWeaponState::Type CurrentState;

	/** Last time this weapon fired successfully */
	float LastFireTime;

	/** Last time this weapon was switched to */
	float EquipStartedTime;

	/** How long does it take to equip the weapon */
	float EquipDuration;

	/** Current total ammo */
	UPROPERTY(Transient)
	int32 CurrentAmmo;

	/** Current ammo in clip */
	UPROPERTY(Transient)
	int32 CurrentAmmoInClip;

	/** Burst fire counter */
	UPROPERTY(Transient)
	int32 BurstCounter;

	/** Handle event of equip finish after timeout */
	FTimerHandle TimerHandle_OnEquipFinished;
	
	/** Handle event for stop reload after timeout */
	FTimerHandle TimerHandle_StopReload;

	/** Handle event for reload weapon after timeout */
	FTimerHandle TimerHandle_ReloadWeapon;

	/** Handle event for firing weapon */
	FTimerHandle TimerHandle_HandleFiring;

	// Animations //

	/** Animation played when equipping the weapon */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim EquipAnim;

	/** Animation played when reloading the weapon */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim ReloadAnim;

	/** Animation played when firing the weapon */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim FireAnim;

	/** is muzzle FX looped? */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	uint32 bLoopedMuzzleFX : 1;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	uint32 bLoopedFireAnim : 1;

	/** impact effects */
	/*UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<AShooterImpactEffect> ImpactTemplate;*/

	/** smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* TrailFX;

	/** param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName TrailTargetParam;

	/** instant hit notify for replication */
	UPROPERTY(Transient)
	FHitInfo HitNotify;

	/** current spread from continuous firing */
	float CurrentFiringSpread;

	// weapon damage //

	/** process the instant hit and notify the server if necessary */
	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** get current spread */
	float GetCurrentSpread() const;

	/** check if weapon should deal damage to actor */
	bool ShouldDealDamage(AActor* TestActor) const;

	/** handle damage */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);

	// Effects //
	/** spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

	/** spawn trail effect */
	void SpawnTrailEffect(const FVector& EndPoint);

private:
	/** First person mesh */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** Third person mesh */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh3P;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	/** Called in network play to do the cosmetic fx for firing */
	virtual void SimulateWeaponFire();

	/** Called in network play to stop cosmetic fx (e.g. for a looping shot). */
	virtual void StopSimulatingWeaponFire();

	// Weapon usage //

	/** Implement weapon firing */
	virtual void FireWeapon();

	/** Handle weapon refiring */
	void HandleRefiring();

	/** Handle weapon firing */
	void HandleFiring();

	/** Burst fire started */
	virtual void OnBurstStarted();

	/** Burst fire finished */
	virtual void OnBurstFinished();

	/** Update weapon state */
	void SetWeaponState(EWeaponState::Type NewState);

	/** Determine current weapon state */
	void CheckCurrentWeaponState();

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

	// Weapon usage helpers //

	/** Play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	/** Play weapon animation */
	float PlayWeaponAnimation(const FWeaponAnim& Animation);

	/** Stop weapon animation */
	void StopWeaponAnimation(const FWeaponAnim& Animation);

	/** Get the aim of the camera */
	FVector GetCameraAim() const;

	/** Get the aim of the weapon, allowing for adjustments to be made by the weapon */
	virtual FVector GetAdjustedAim() const;

	/** get the originating location for camera damage */
	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	/** Get the muzzle location of the weapon */
	FVector GetMuzzleLocation() const;

	/** Get muzzle direction */
	FVector GetMuzzleDirection() const;

	/** find hit */
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

protected:
	/** Returns Mesh1P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns Mesh3P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
};
