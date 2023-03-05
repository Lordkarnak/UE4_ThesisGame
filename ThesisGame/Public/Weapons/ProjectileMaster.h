// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponProjectile.h"
#include "ProjectileMaster.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS(Blueprintable)
class AProjectileMaster : public AActor
{
	GENERATED_UCLASS_BODY()

	/** Projectile collision */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Collision)
	class USphereComponent* CollisionComponent;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* MovementComponent;

	/** Projectile particles */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UParticleSystemComponent* ParticleComponent;
	
public:	
	// Sets default values for this actor's properties
	//AProjectileMaster();

	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** Calculate velocity of projectile based on direction */
	void InitVelocity(FVector& ShootDir);

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** called when projectile stops moving */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

	/** effects for explosion */
	//UPROPERTY(EditDefaultsOnly, Category = Effects)
	//TSubclassOf<class AShooterExplosionEffect> ExplosionTemplate;
	
	UPROPERTY(Transient)
	bool bExploded;

	UPROPERTY(Transient)
	bool bPendingDestroy;

	/** Show explosion effects, used when projectile has radial damage */
	void Explode(const FHitResult& Impact);

	/** Cleanup projectile and destroy it */
	void DisableAndDestroy();

	/** projectile settings given by the weapon */
	struct FWeaponProjectileData ProjectileConfig;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

protected:
	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetProjectileCollision() const { return CollisionComponent; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return MovementComponent; }
	/** Returns ParticleComp subobject **/
	FORCEINLINE UParticleSystemComponent* GetParticleComponent() const { return ParticleComponent; }
};
