// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "AudioThread.h"
#include "ThesisGameHUD.h"
#include "Characters/PlayableController.h"
#include "PlayableCharacter.generated.h"

class UInputComponent;
class USphereComponent;
class UBoxComponent;
class UAnimMontage;
class USoundCue;
class UAIPerceptionStimuliSourceComponent;
class UThesisGameInstance;
struct FActorData;

USTRUCT()
struct FWeaponAnims
{
	GENERATED_USTRUCT_BODY()

	/** 1st person anim played on this character */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Anim1P;

	/** 3rd person anim played on this char */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Anim3P;
};


UCLASS()
class APlayableCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/*UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh3P;*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera3P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = InteractionVolume, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* InteractionCollisionComp;

public:

	// Play animations on this character
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	// Stop playing an animation on this character
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/** stop playing all montages */
	void StopAllAnimMontages();

	//***********
	// Movement properties

	/** Base turn rate, in deg/sec. Final turn rate is affected by scaling. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Final turn rate is affected by scaling. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/** Property used to differentiate actors placed in level. When loaded from a save, this actor will take data using his ID.*/
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Game|Character")
	int32 ActorID;
	

protected:
	// Setup character variables on runtime
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Clean up character
	virtual void BeginDestroy() override;

	virtual void Destroyed() override;

	/** update mesh for first person view */
	virtual void PawnClientRestart() override;

	// What happens when character takes damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** Returns True if the pawn can die in the current state */
	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	/**
	* Kills pawn.
	* @param KillingDamage - Damage amount of the killing blow
	* @param DamageEvent - Damage event of the killing blow
	* @param Killer - Who killed this pawn
	* @param DamageCauser - the Actor that directly caused the damage (i.e. the Projectile that exploded, the Weapon that fired, etc)
	* @returns true if allowed
	*/
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	// What happens when character falls out of boundaries
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	//****************
	//Movement Events

	// Handle moving forward/backward
	void MoveForward(float Value);

	// Handle moving left/right
	void MoveRight(float Value);

	/**
	* Called via input to turn on the X/-X axis at a given rate
	* @param Rate Normalized rate in range (0.0, 1.0)
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn on the Y/-Y axis at a given rate
	* @param Rate Normalized rate in range (0.0, 1.0)
	*/
	void LookUpAtRate(float Rate);

	//*****************
	//Action Events

	/** Handle sprinting/normal running */
	void OnSprintStart();

	/** Handle sprinting stop */
	void OnSprintStop();

	/** Long range hit ,fire the currently equipped weapon */
	void OnFireStart();

	/** Stop firing weapon */
	void OnFireStop();

	/** Equip next weapon */
	void OnNextWeapon();

	/** Equip previous weapon */
	void OnPrevWeapon();

	/** Close range hit, attack with currently equipped weapon */
	void OnMelee();

	/** Reload currently equipped weapon */
	void OnReload();

	/** Use weapon's iron sights, focus on target */
	void OnIronSightsStart();

	/** Stop using iron sights, lose focus */
	void OnIronSightsStop();

	void RestartPlayer();

	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/** Display interaction UI and enable extra controls */
	UFUNCTION()
	void OnInteractionCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Stop interacting */
	UFUNCTION()
	void OnInteractionCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//*****************
	//Animation helper functions
	
	//*****************
	//Inventory Events

	/** Fill character's inventory with a predefined set of classes */
	void SpawnDefaultInventory();

	/** Fill character's inventory using a SaveGame actor data */
	void SpawnLoadedInventory(const FActorData& ActorData);

	/** Remove all weapons from inventory and destroy the reference */
	void DestroyInventory();

	/** Add weapon to inventory */
	void AddWeapon(class AWeaponMaster* Weapon);

	/** Remove weapon from inventory */
	void RemoveWeapon(class AWeaponMaster* Weapon);

	/** Search in inventory */
	class AWeaponMaster* FindWeapon(TSubclassOf<class AWeaponMaster> WeaponClass);

	/** Equip selected weapon */
	void EquipWeapon(class AWeaponMaster* NewWeapon);

	//********************
	//Startup updates

	/** Load defaults from Save file */
	UFUNCTION()
	virtual void LoadCharacterDefaults();

	/** handles sounds for running */
	void UpdateRunSounds();

	/** handle mesh visibility and updates */
	void UpdatePawnMeshes();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called on every tick
	//virtual void UpdateCamera(float DeltaTime) override;

	//*****************
	//Utilities
	/** play effects on hit */
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	/** switch to ragdoll */
	void SetRagdollPhysics();

	//*****************
	//Getters

	/** get targeting state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsTargeting() const;

	/** get firing state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsFiring() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Character")
	bool IsSprinting() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Character")
	bool IsRunning() const;

	/** Get character's base health */
	int32 GetBaseHealth() const;

	/** Get character's current health */
	UFUNCTION(BlueprintCallable, Category = "Game|Character")
	int32 GetHealth() const;

	/** Get character's current health as percentage */
	UFUNCTION(BlueprintCallable, Category = "Game|Character")
	float GetHealthPercentage() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Character")
	TArray<class AWeaponMaster*> GetCurrentInventory() const;

	class AWeaponMaster* GetCurrentWeapon() const;

	/** See if character is alive and healthy */
	UFUNCTION(BlueprintCallable, Category = "Game|Character")
	bool IsAlive() const;

	USkeletalMeshComponent* GetPawnMesh() const;

	/** Alter character position according to camera pitch*/
	void OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation);

	/** get aim offsets */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	UFUNCTION(BlueprintCallable, Category = Mesh)
	virtual bool IsFirstPerson() const;

	bool IsEnemyFor(AController* TestPC) const;

	/** Get character's currently equipped weapon as reference */
	UFUNCTION(BlueprintCallable, Category = "Game|Inventory")
	class AWeaponMaster* GetWeapon() const;

	/** Find a bone/socket to attach equipped weapon to */
	FName GetWeaponSocket() const;

	UAnimMontage* GetEquipAnim() const;

	UAnimMontage* GetReloadAnim() const;

	UAnimMontage* GetFireAnim() const;

	UFUNCTION(BlueprintCallable)
	bool CanPickup() const;

	/** Get current ID assigned to this actor. Used for saving individual bots */
	int32 GetID() const;

protected:
	//*****************
	//Character HUD

	/** Character's health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	float fCharacterHealth;

	/** When health is considered low, usually around 15% */
	float LowHealthPercentage;

	UPROPERTY(EditDefaultsOnly, Category = HUD)
	UForceFeedbackEffect* HitForceFeedback;

	UPROPERTY(EditDefaultsOnly, Category = HUD)
	UForceFeedbackEffect* KilledForceFeedback;

	//*****************
	//Character inventory

	/** Default inventory list */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<class AWeaponMaster>> DefaultInventoryClasses;

	/** Inventory */
	UPROPERTY(Transient)
	TArray<class AWeaponMaster*> CharacterInventory;

	//*****************
	//Character weapon

	/** Currently equipped weapon */
	UPROPERTY(Transient)
	class AWeaponMaster* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName WeaponSocket;

	//*****************
	//Character audio

	/** sound played on death, local player only */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* DeathSound;

	/** sound played when health is low */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* LowHealthSound;

	/** sound played when running */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* RunLoopSound;

	/** sound played when stop running */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* RunStopSound;

	/** sound to use when using iron sights */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* TargetingSound;

	UPROPERTY()
	UAudioComponent* RunLoopAC;

	//*****************
	//Character states booleans

	/** Determine if character is dying. Used to play proper effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character States")
	uint32 bIsDying : 1;

	/** Determine if character is using iron sights */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character States")
	uint8 bIsTargeting : 1;

	/** Determine if character is firing a weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character States")
	uint8 bIsFiring : 1;

	/** Determine if character is sprinting */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character States")
	uint8 bIsSprinting : 1;

	/** Determine if character is crouching */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character States")
	uint8 bIsCrouching : 1;

	/** Determine if character is proning to the ground */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character States")
	uint8 bIsProning : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character Menu")
	bool bIsViewingQuestLog;

	UPROPERTY(BlueprintReadOnly, Category = "Character States")
	bool bCanPickup;

	//*****************
	//Animations

	/** Animation played when equipping the weapon */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnims EquipAnim;

	/** Animation played when reloading the weapon */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnims ReloadAnim;

	/** Animation played when firing the weapon */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnims FireAnim;

	/** Animation played when melee attacking with this weapon */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnims MeleeAnim;

	/** Character extra animations, forced via script */
	UPROPERTY(EditDefaultsOnly, Category = Animations)
	UAnimMontage* DeathAnim;

	/** Modifies base character speed, if aiming or sprinting */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Character")
	float fSpeedRatio;

	/** Preset walking speed modifier */
	float fWalkingSpeed;

	/** Preset firing speed modifier */
	float fFiringSpeed;

	/** Preset targeting speed modifier */
	float fTargetingSpeed;

	/** Used to alter speed per frame */
	bool bPendingSpeedChange;

	/** Protect controller reference to use it when restarting this pawn */
	AController* LateControllerReference;

	/** Pointer to the current game instance, used to access game data */
	UThesisGameInstance* CurrentGameInstance;

	//******************
	//AI Stimuli
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Stimuli")
	UAIPerceptionStimuliSourceComponent* StimuliSourceComp;

public:
	/** Return first person mesh */
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Return third person mesh */
	/*FORCEINLINE class USkeletalMeshComponent* GetMesh3P() const { return Mesh; }*/
	/** Return first person camera */
	FORCEINLINE class UCameraComponent* GetCamera1P() const { return Camera1P; }
	/** Return first person camera */
	/*FORCEINLINE class UCameraComponent* GetCamera3P() const { return Camera3P; }*/

};