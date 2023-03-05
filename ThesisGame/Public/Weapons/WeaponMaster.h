// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayableCharacter.h"
#include "WeaponMaster.generated.h"

class UAnimMontage;
class APlayableCharacter;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShake;
class UForceFeedbackEffect;
class USoundCue;
class AWeaponImpact;
class UPawnNoiseEmitterComponent;
struct FInventoryData;

namespace EWeaponStates
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Equipping
	};
}

USTRUCT()
struct FWeaponProperties
{
	GENERATED_USTRUCT_BODY()

	//*****************
	//Ammo properties

	/** Max ammo a weapon has */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxAmmo;

	/** Ammo in clip */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxAmmoInClip;

	/** Start the game with that many ammo */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 InitialAmmo;

	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	bool HasInfiniteAmmo;

	//*****************
	//Animation properties

	/** Time between consecutive shots */
	UPROPERTY(EditDefaultsOnly, Category = Properties)
	float fShotTimeout;

	/** Default equip duration if no animations are being played */
	UPROPERTY(EditDefaultsOnly, Category = Properties)
	float fDefaultEquipDuration;

	/** Default reload duration if no animations are being played */
	UPROPERTY(EditDefaultsOnly, Category = Properties)
	float fDefaultReloadDuration;

	//******************
	//Spread properties

	/** Fire spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float FiringSpread;

	/** Fire spread when using iron sights (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float IronSightsSpreadModifier;

	/** Fire spread when a bot is using the weapon, used to balance difficulty */
	/*UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float BotSpread;*/

	/** Fire spread fine tuning (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float FiringSpreadModifier;

	/** Fire spread limit to avoid spreading idefinetely (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float FiringSpreadLimit;

	//******************
	//weapon properties

	/** How far does this weapon shoots */
	UPROPERTY(EditDefaultsOnly, Category = Properties)
	float Reach;

	/** How much damage it deals */
	UPROPERTY(EditDefaultsOnly, Category = Properties)
	float Firepower;

	/** type of damage it deals */
	UPROPERTY(EditDefaultsOnly, Category = Properties)
	TSubclassOf<UDamageType> DamageType;

	/** hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
	float AllowedViewDotHitDir;

	UPROPERTY(EditDefaultsOnly, Category = Properties)
	bool CanTarget;

	FWeaponProperties()
	{
		MaxAmmo = 100;
		MaxAmmoInClip = 20;
		InitialAmmo = 40;
		HasInfiniteAmmo = false;

		fShotTimeout = 0.2f;
		fDefaultEquipDuration = 0.5f;
		fDefaultReloadDuration = 1.0f;

		FiringSpread = 2.0f;
		IronSightsSpreadModifier = 0.25f;
		FiringSpreadModifier = 1.0f;
		FiringSpreadLimit = 10.0f;

		Reach = 10000.0f;
		Firepower = 10.0f;
		DamageType = UDamageType::StaticClass();
		AllowedViewDotHitDir = 0.8f;

		CanTarget = true;
	}
};

USTRUCT()
struct FHitProperties
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		FVector Origin;

	UPROPERTY()
		float ReticleSpread;

	UPROPERTY()
		int32 RandomSeed;
};

UCLASS(Blueprintable)
class AWeaponMaster : public AActor
{
	GENERATED_UCLASS_BODY()

public:	
	// Sets default values for this actor's properties
	AWeaponMaster();

	//******************
	//Events

	/** Event called when a pawn tries to equip a weapon. */
	virtual void OnEquip(const AWeaponMaster* LastWeapon);

	/** Finish equip event */
	virtual void OnEquipFinished();

	/** Event called when a pawn holsters a weapon */
	virtual void OnUnEquip();

	/** Event called when a weapon is added to a pawn's inventory */
	virtual void OnEnterInventory(APlayableCharacter* NewOwner);

	/** Event called when a weapon leaves the inventory */
	virtual void OnLeaveInventory();

	/** start firing burst */
	virtual void OnBurstStarted();

	/** stop firing burst */
	virtual void OnBurstFinished();

	//******************
	//Utilities

	/** Add ammo to the weapon */
	int32 AddAmmo(int Amount);

	/** Remove ammo from the weapon */
	void RemoveAmmo(int Amount);

	/** Modify the weapon to infinite rounds. Useful for bots */
	void SetInfiniteAmmo(bool bInfinite);

	/** Start firing */
	virtual void StartFire();

	/** Stop firing */
	virtual void StopFire();

	/** Start reloading */
	virtual void StartReload();

	/** Perform reload logic */
	virtual void Reload();

	/** Stop reloading */
	virtual void StopReload();

	/** Start iron sights targeting */
	virtual void StartIronSights();

	/** Stop using iron sights */
	virtual void StopIronSights();

	/**  Start using melee attack */
	virtual void StartMelee();

	/** set the owner of this weapon */
	void SetOwningPawn(APlayableCharacter* NewOwner);

	/** Load weapon defaults using the inventory struct */
	void LoadWeaponData(const FInventoryData& Data);

	//*****************
	//Boolean checks

	/** Is this weapon equipped by a pawn? */
	bool IsEquipped() const;

	/** Is this weapon attached to a pawn? */
	bool IsAttachedToPawn() const;

	/** Can this weapon fire? */
	bool CanFire() const;

	/** Can this weapon reload? */
	bool CanReload() const;

	/** get current spread */
	float GetCurrentSpread() const;

	/** check if weapon should deal damage to target */
	bool ShouldDealDamage(AActor* TestActor) const;

	/** Do damage to enemy pawn */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);

	/** gets last time this weapon was switched to */
	float GetEquipStartedTime() const;

	/** gets animation duration of equip */
	float GetEquipDuration() const;

	/** gets the current weapon state */
	EWeaponStates::Type GetCurrentState() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	virtual void Destroyed() override;

	//******************
	//Utilities

	/** process the instant hit and notify the server if necessary */
	virtual void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	//*********************
	//Effects events

	/** Called in network play to do the cosmetic fx for firing */
	virtual void SimulateWeaponFire();

	/** Called in network play to stop cosmetic fx (e.g. for a looping shot). */
	virtual void StopSimulatingWeaponFire();

	/** spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

	/** spawn trail effect */
	void SpawnTrailEffect(const FVector& EndPoint);

	//*****************
	//Setters

	/** Update weapon state */
	void SetWeaponState(EWeaponStates::Type NewState);

	void DetermineWeaponState();

	//*****************
	//Config

	/** Who holds this weapon */
	UPROPERTY(Transient)
	class APlayableCharacter* MyPawn;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FWeaponProperties WeaponConfig;

	/** Name of muzzle bone/socket */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FName MuzzleSocket;

	//*****************
	//Effects

	/** Muzzle flash FX */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* MuzzleFX;

	/** Component for muzzle flash */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	/** Component for third person muzzle flash */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSCSecondary;

	/** Camera shake effect when firing */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<UCameraShake> FireCameraShake;

	/** Force feedback effect to play when firing */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UForceFeedbackEffect* FireForceFeedback;

	/** impact effect */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<AWeaponImpact> ImpactTemplate;

	/** smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* TrailFX;

	/** param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName TrailTargetParam;	

	//*****************
	//Audio

	/** Firing audio */
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

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

	/** Equip sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* TargetingSound;

	//*****************
	//Boolean states

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

	//*******************
	//Weapon info

	/** Current weapon state*/
	EWeaponStates::Type CurrentState;

	/** Last time this weapon fired successfully */
	float LastFireTime;

	/** Last time this weapon was switched to */
	float EquipStartedTime;

	/** How long does it take to equip the weapon */
	float EquipDuration;

	/** Current ammo, used to fire */
	UPROPERTY(Transient)
	int32 CurrentAmmo;

	/** Current max ammo, used to reload */
	UPROPERTY(Transient)
	int32 CurrentMaxAmmo;

	/** current spread from continuous firing */
	float CurrentFiringSpread;

	float CurrentFirepower;

	/** Adjustment to handle frame rate affecting actual timer interval. */
	UPROPERTY(Transient)
	float TimerIntervalAdjustment;

	/** Whether to allow automatic weapons to catch up with shorter refire cycles */
	UPROPERTY(Config)
	bool bAllowAutomaticWeaponCatchup = true;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	bool bDebug = true;

	//****************
	//Timer handlers

	/** Handle event of equip finish after timeout */
	FTimerHandle TimerHandle_OnEquipFinished;

	/** Handle event for stop reload after timeout */
	FTimerHandle TimerHandle_StopReload;

	/** Handle event for reload weapon after timeout */
	FTimerHandle TimerHandle_ReloadWeapon;

	/** Handle event for firing weapon */
	FTimerHandle TimerHandle_HandleFiring;

	//******************
	//AI Stimuli
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Stimuli")
	UPawnNoiseEmitterComponent* PawnNoiseEmitterComp;

private:
	/** First person mesh */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** Third person mesh */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh3P;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Implement weapon firing */
	virtual void FireWeapon();

	/** Handle weapon refiring */
	void HandleRefiring();

	/** Handle weapon firing */
	void HandleFiring();

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

	/** find hit */
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

public:

	//******************
	//Getters

	/** get max ammo this weapon can carry */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	int32 GetMaxAmmo() const;

	/** get max ammo a clip can hold */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	int32 GetAmmoInClip() const;

	/** get current ammo in clip */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	int32 GetCurrentAmmo() const;

protected:
	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class APlayableCharacter* GetPawnOwner() const;

	USkeletalMeshComponent* GetWeaponMesh() const;

	/** Get the aim of the weapon, allowing for adjustments to be made by the weapon */
	virtual FVector GetAdjustedAim() const;

	/** Get the aim of the camera */
	FVector GetCameraAim() const;

	/** get the originating location for camera damage */
	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	/** Get the muzzle location of the weapon */
	FVector GetMuzzleLocation() const;

	/** Get muzzle direction */
	FVector GetMuzzleDirection() const;

	//*****************
	//Audio

	/** Play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	//*****************
	//Animations

	/** Play weapon animation */
	float PlayWeaponAnimation(UAnimMontage* Animation);

	/** Stop weapon animation */
	void StopWeaponAnimation(UAnimMontage* Animation);

protected:
	/** Returns Mesh1P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns Mesh3P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
};
