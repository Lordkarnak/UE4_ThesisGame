// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponProjectile.h"
#include "ProjectileMaster.h"

AWeaponProjectile::AWeaponProjectile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void AWeaponProjectile::ApplyProjectileConfig(FWeaponProjectileData& Data)
{
	Data = WeaponProjectileConfig;
}

/*void AWeaponProjectile::FireWeapon()
{
	if (WeaponProjectileConfig.ProjectileClass != NULL)
	{
		FVector ShootDir = GetAdjustedAim();
		FVector Origin = GetMuzzleLocation();

		const float ProjectileRange = 10000.0f;
		const FVector StartTrace = GetCameraDamageStartLocation(ShootDir);
		const FVector EndTrace = StartTrace + ShootDir * ProjectileRange;
		FHitResult Impact = WeaponTrace(StartTrace, EndTrace);

		if (Impact.bBlockingHit)
		{
			const FVector AdjustedDir = (Impact.ImpactPoint - Origin).GetSafeNormal();
			bool bWeaponPenetration = false;

			const float DirectionDot = FVector::DotProduct(AdjustedDir, ShootDir);
			if (DirectionDot < 0.0f)
			{
				// shooting backwards = weapon is penetrating
				bWeaponPenetration = true;
			}
			else if (DirectionDot < 0.5f)
			{
				// check for weapon penetration if angle difference is big enough
				// raycast along weapon mesh to check if there's blocking hit

				FVector MuzzleStartTrace = Origin - GetMuzzleDirection() * 150.0f;
				FVector MuzzleEndTrace = Origin;
				FHitResult MuzzleImpact = WeaponTrace(MuzzleStartTrace, MuzzleEndTrace);
			
				bWeaponPenetration = MuzzleImpact.bBlockingHit;
			}

			if (bWeaponPenetration)
			{
				Origin = Impact.ImpactPoint - ShootDir * 10.0f;
			}
			else
			{
				ShootDir = AdjustedDir;
			}
		}

		FireProjectile(Origin, ShootDir);
	}
}*/

void AWeaponProjectile::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	//Super::ProcessInstantHit(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
	/*const FVector EndTrace = Origin + ShootDir * WeaponConfig.Reach;
	const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;*/
	//FireProjectile(Origin, ShootDir);

	// play FX
	const FVector EndTrace = Origin + ShootDir * WeaponConfig.Reach;
	const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;
	FVector EndTraceDirection = EndPoint;

	SpawnTrailEffect(EndPoint);
	SpawnImpactEffects(Impact);
	SpawnProjectile(EndTrace);
}

void AWeaponProjectile::SpawnProjectile(const FVector& ProjectileDirection)
{
	if (WeaponProjectileConfig.ProjectileClass != NULL)
	{
		//FVector EndTrace = Origin + ShootDir * WeaponConfig.Reach;
		//const FRotator SpawnRotation = EndTrace.Rotation();
		//const FRotator SpawnRotation = Impact.ImpactNormal.Rotation();
		//FRotator SpawnWorldRotation;
		//FVector SpawnWorldLocation;
		//GetWeaponMesh()->GetSocketWorldLocationAndRotation(MuzzleSocket, SpawnWorldLocation, SpawnWorldRotation);
		//FRotator MuzzleLocalRotation = GetWeaponMesh()->GetSocketRotation(MuzzleSocket);
		//FRotator WeaponLocalRotation = GetWeaponMesh()->GetRelativeRotation();
		
		//SpawnWorldRotation = FRotator(FQuat(MuzzleLocalRotation) * FQuat(WeaponLocalRotation));
		//SpawnWorldRotation = MuzzleLocalRotation.RotateVector(EndPoint).Rotation();

		//const FVector SpawnLocation = GetMuzzleLocation();
		//WeaponTrace(SpawnLocation, EndTrace);

		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FVector SpawnOffset = FVector(100.f, 0.0f, 0.0f);
			const FRotator SpawnRotation = GetPawnOwner()->GetControlRotation();
			const FVector SpawnLocation = (!MuzzleSocket.IsNone()) ? GetMuzzleLocation() : GetWeaponMesh()->GetComponentLocation() + SpawnRotation.RotateVector(SpawnOffset);

			FActorSpawnParameters SpawnParams;
			AProjectileMaster* Projectile = World->SpawnActor<AProjectileMaster>(WeaponProjectileConfig.ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

			if (Projectile)
			{
				Projectile->SetInstigator(GetInstigator());
				Projectile->SetOwner(this);
				FVector EndDirection = SpawnRotation.Vector();
				Projectile->InitVelocity(EndDirection);
			}
		}
	}
}