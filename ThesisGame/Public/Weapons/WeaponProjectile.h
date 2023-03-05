// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponMaster.h"
#include "WeaponProjectile.generated.h"

USTRUCT()
struct FWeaponProjectileData
{
	GENERATED_USTRUCT_BODY()

	/** projectile class */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AProjectileMaster> ProjectileClass;

	/** projectile lifetime */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	float ProjectileLifetime;

	/** Radial damage, if any */
	UPROPERTY(EditDefaultsOnly, Category=DamageStats)
	int32 ProjectileDamage;

	/** Radius of impact, if any. Zero means direct hit */
	UPROPERTY(EditDefaultsOnly, Category=DamageStats)
	float ImpactRadius;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category=DamageStats)
	TSubclassOf<UDamageType> DamageType;

	/** defaults*/
	FWeaponProjectileData()
	{
		ProjectileClass = NULL;
		ProjectileLifetime = 3.0f;
		ProjectileDamage = 0;
		ImpactRadius = 0.0f;
		DamageType = UDamageType::StaticClass();
	}
};

/**
 * 
 */
UCLASS()
class THESISGAME_API AWeaponProjectile : public AWeaponMaster
{
	GENERATED_UCLASS_BODY()

	/** Apply projectile settings to given projectile */
	void ApplyProjectileConfig(FWeaponProjectileData& Data);

protected:

	UPROPERTY(EditDefaultsOnly, Category=Config)
	FWeaponProjectileData WeaponProjectileConfig;

	/** spawn a projectile on top of other effects */
	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread) override;

	UFUNCTION(BlueprintCallable)
	void SpawnProjectile(const FVector& ProjectileDirection);
};
