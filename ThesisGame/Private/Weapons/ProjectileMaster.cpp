// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileMaster.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectileMaster::AProjectileMaster(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Use a sphere as a simple collision representation
	CollisionComponent = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	CollisionComponent->InitSphereRadius(5.0f);
	CollisionComponent->AlwaysLoadOnClient = true;
	CollisionComponent->AlwaysLoadOnServer = true;
	CollisionComponent->bTraceComplexOnMove = false;
	CollisionComponent->BodyInstance.SetCollisionProfileName("Projectile");
	/*CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);*/

	// Players can't walk on it
	CollisionComponent->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComponent->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComponent;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	MovementComponent = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	MovementComponent->UpdatedComponent = CollisionComponent;
	MovementComponent->InitialSpeed = 3000.f;
	MovementComponent->MaxSpeed = 3000.f;
	//MovementComponent->ProjectileGravityScale = 0.0f;
	MovementComponent->bRotationFollowsVelocity = true;
	MovementComponent->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// Init properties
	bPendingDestroy = false;
	bExploded = false;
}

void AProjectileMaster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Explode projectile if it's a bomb, otherwise just hit
	/*if (ProjectileConfig.ImpactRadius > 0)
	{
		MovementComponent->OnProjectileStop.AddDynamic(this, &AProjectileMaster::OnImpact);
	}*/
	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectileMaster::OnHit);
	CollisionComponent->MoveIgnoreActors.Add(GetInstigator());

	AWeaponProjectile* OwnerWeapon = Cast<AWeaponProjectile>(GetOwner());
	if (OwnerWeapon)
	{
		OwnerWeapon->ApplyProjectileConfig(ProjectileConfig);
	}

	SetLifeSpan(ProjectileConfig.ProjectileLifetime);
}

void AProjectileMaster::InitVelocity(FVector& ShootDir)
{
	if (MovementComponent)
	{
		MovementComponent->Velocity = ShootDir * MovementComponent->InitialSpeed;
	}
}

// Called when the game starts or when spawned
//void AProjectileMaster::BeginPlay()
//{
//	Super::BeginPlay();
//}

// Called every frame
//void AProjectileMaster::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void AProjectileMaster::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 50.0f, GetActorLocation());
		AWeaponProjectile* OwnerWeapon = Cast<AWeaponProjectile>(GetOwner());
		if (!bPendingDestroy)
		{
			DisableAndDestroy();
		}
	}
}

void AProjectileMaster::OnImpact(const FHitResult& HitResult)
{
	if (!bExploded)
	{
		Explode(HitResult);
		if (!bPendingDestroy)
		{
			DisableAndDestroy();
		}
	}
}

void AProjectileMaster::Explode(const FHitResult& Impact)
{
	if (ParticleComponent)
	{
		ParticleComponent->Deactivate();
	}

	const FVector ImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;

	if (ProjectileConfig.ProjectileDamage > 0 && ProjectileConfig.ImpactRadius > 0 && ProjectileConfig.DamageType)
	{
		UGameplayStatics::ApplyRadialDamage(this, ProjectileConfig.ProjectileDamage, ImpactLocation, ProjectileConfig.ImpactRadius, ProjectileConfig.DamageType, TArray<AActor*>(), this, GetInstigatorController());
	}

	/*if (ExplosionTemplate)
	{
		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), ImpactLocation);
		AActor* const EffectActor = GetWorld()->SpawnActorDeferred<AActor>(ExplosionTemplate, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}
	}*/

	bExploded = true;
}

void AProjectileMaster::DisableAndDestroy()
{
	bPendingDestroy = true;

	UAudioComponent* ProjectileAudioComponent = FindComponentByClass<UAudioComponent>();
	if (ProjectileAudioComponent && ProjectileAudioComponent->IsPlaying())
	{
		ProjectileAudioComponent->FadeOut(0.1f, 0.0f);
	}

	MovementComponent->StopMovementImmediately();

	SetLifeSpan(2.0f);
}