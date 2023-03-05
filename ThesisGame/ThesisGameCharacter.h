// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AudioThread.h"
#include "ThesisGame.h"
#include "ThesisGameGameMode.h"
#include "ThesisGameCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AThesisGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

public:
	AThesisGameCharacter();
	
	// Read Weapon Data //

	/** Get current weapon */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class AWeaponBase* GetWeapon() const;

	// Weapon Usage //

	/** Find a named bone/socket to attach currently equipped weapon to */
	FName GetWeaponAttachPoint() const;

	/** Add weapon to inventory */
	void AddWeapon(class AWeaponBase* Weapon);

	/** Remove weapon from inventory */
	void RemoveWeapon(class AWeaponBase* Weapon);

	/** Find this weapon in inventory */
	class AWeaponBase* FindWeapon(TSubclassOf<class AWeaponBase> WeaponClass);

	/** Equip this weapon */
	void EquipWeapon(class AWeaponBase* Weapon);

	/** Start firing */
	void StartWeaponFire();

	/** Stop firing */
	void StopWeaponFire();

	/** Check if pawn is able to fire a weapon */
	bool CanFire() const;

	/** Check if pawn can reload a weapon */
	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsFiring() const;

	// Alive/Dead state //

	/** Get pawn's max health pool */
	int32 GetMaxHealth() const;

	/** Check if pawn is still alive */
	bool IsAlive() const;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health)
	uint32 bIsDying : 1;

	/** Calculate health and handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	/** Event that occurs when pawn has taken enough damage to be killed */
	//virtual bool Kill();

	/** True if the pawn can die in the current state */
	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCause) const;

	/** Ensure death when this pawn gets out of bounds */
	virtual void OutOfBoundsKill(const class UDamageType& dmgType);

	/** Kill this pawn */
	virtual bool Kill(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	/** Event that occurs when this pawn is killed */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/** Play an effect on hit */
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/** Do ragdoll when dead */
	void SetRagdollPhysics();

	/** updates current weapon */
	void SetCurrentWeapon(class AWeaponBase* NewWeapon, class AWeaponBase* LastWeapon = NULL);

	/** [server] spawns default inventory */
	void SpawnDefaultInventory();

	/** [server] remove all weapons from inventory and destroy them */
	void DestroyInventory();

	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/** stop playing all montages */
	void StopAllAnimMontages();

protected:
	virtual void BeginPlay();

	/** initialize character */
	virtual void PostInitializeComponents() override;

	virtual void Destroyed() override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AThesisGameProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	float fActorHealth;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Fire weapon on action press */
	void OnStartFire();

	/** Stop firing weapon on action release */
	void OnStopFire();

	/** Cycles weapon right on action press */
	void OnNextWeapon();

	/** Cycles weapon left on action press */
	void OnPrevWeapon();

	/** Reload weapon on action press */
	void OnReload();

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

	/** socket/bone to attach weapon to */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName WeaponAttachPoint;

	/** inventory list */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<class AWeaponBase>> DefaultInventoryClasses;

	/** Inventory */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<class AWeaponBase*> Inventory;

	// Weapon Data //
	UPROPERTY(Transient)
	class AWeaponBase* CurrentWeapon;

	/** current firing state */
	uint8 bIsFiring : 1;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	UAnimMontage* DeathAnim;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* DeathSound;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* LowHealthSound;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RunLoopSound;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RunStopSound;

	/** used to manipulate with run loop sound */
	UPROPERTY()
	UAudioComponent* RunLoopAC;

	/** hook to looped low health sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowHealthWarningPlayer;

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

