// Fill out your copyright notice in the Description page of Project Settings.


#include "BotAIController.h"
#include "Characters/PlayableController.h"
#include "Characters/PlayableCharacter.h"
#include "Characters/BotCharacter.h"
#include "Weapons/WeaponMaster.h"
#include "GameFramework/GameModeBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AIPerceptionTypes.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

ABotAIController::ABotAIController()
{
	// Init affiliation
	SetGenericTeamId(FGenericTeamId(2));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	BrainComponent = BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 3000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 2000.0f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->ConfigureSense(*HearingConfig);
	
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

	bWantsPlayerState = true;
}

//Overrides
//void ABotAIController::BeginPlay()
//{
//
//}

ETeamAttitude::Type ABotAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* OtherPawn = Cast<APawn>(&Other);
	if (OtherPawn)
	{
		
		/*ABotAIController* OtherAIController = Cast<ABotAIController>(OtherPawn->GetController());
		APlayableController* OtherPlayerController = Cast<APlayableController>(OtherPawn->GetController());*/

		const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController());
		if (TeamAgent)
		{
			uint8 OtherTeamID = TeamAgent->GetGenericTeamId().GetId();
			UE_LOG(LogTemp, Warning, TEXT("My team ID %d, Class %s | Other team ID %d, Class %s"), GetGenericTeamId().GetId(), *this->GetClass()->GetName(), OtherTeamID, *OtherPawn->GetClass()->GetName());
			
			if (OtherTeamID == 0) // is player team, decide hostile
			{
				UE_LOG(LogTemp, Warning, TEXT("Team Player. Decided hostile"));
				return ETeamAttitude::Hostile;
			}
			else if (OtherTeamID == GetGenericTeamId().GetId())
			{
				UE_LOG(LogTemp, Warning, TEXT("Team Bot. Decided neutral"));
				return ETeamAttitude::Neutral;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Decided whatever."));
			return Super::GetTeamAttitudeTowards(*OtherPawn->GetController());
		}
		/*if (OtherAIController)
		{
			OtherTeamID = OtherAIController->GetGenericTeamId();
		}
		else if (OtherPlayerController)
		{
			OtherTeamID = OtherPlayerController->GetGenericTeamId();
		}*/
		/*else
		{
			const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController());
			if (TeamAgent)
			{
				OtherTeamID = TeamAgent->GetGenericTeamId();
			}
		}*/
	}
	return ETeamAttitude::Neutral;
}

void ABotAIController::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	if (InTeamID != NULL)
	{
		AITeamId = InTeamID;
	}
}

FGenericTeamId ABotAIController::GetGenericTeamId() const
{
	return AITeamId;
}

void ABotAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABotCharacter* Bot = Cast<ABotCharacter>(InPawn);

	//behavior
	if (Bot && Bot->BotBehavior)
	{
		if (Bot->BotBehavior->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*Bot->BotBehavior->BlackboardAsset);
		}

		EnemyKeyID = BlackboardComp->GetKeyID("Enemy");
		NeedAmmoKeyID = BlackboardComp->GetKeyID("NeedAmmo");
		SeenActorKeyID = BlackboardComp->GetKeyID("SeenActor");
		SeenStimuliKeyID = BlackboardComp->GetKeyID("SeenStimuliLocation");
		HearingStimuliKeyID = BlackboardComp->GetKeyID("HearingStimuliLocation");

		BehaviorComp->StartTree(*(Bot->BotBehavior));
	}
	//UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, SightConfig->GetSenseImplementation(), GetPawn());
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABotAIController::OnTargetSenseUpdated);
}

void ABotAIController::OnUnPossess()
{
	Super::OnUnPossess();

	BehaviorComp->StopTree();

	PerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &ABotAIController::OnTargetSenseUpdated);
}

void ABotAIController::OnTargetSenseUpdated(AActor* TargetActor, struct FAIStimulus Stimulus)
{
	APlayableCharacter* PC = Cast<APlayableCharacter>(TargetActor);
	TSubclassOf<UAISense> StimulusClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
	if (StimulusClass == UAISense_Sight::StaticClass())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Saw an actor!"));
		if (TargetActor != NULL)
		{
			SetSeenTarget(TargetActor, Stimulus.StimulusLocation);
			LastSeenLocation = Stimulus.StimulusLocation;
			if (PC)
			{
				SetEnemy(PC);
			}
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Heard a noise!"));
		if (PC)
		{
			SetHeardTarget(TargetActor, TargetActor->GetActorLocation());
		}
	}
	//int32 TargetsLength = NewTargets.Num();
	////SetSensedTarget(NewTargets[0], NewTargets[0]->GetActorLocation());
	//for (int i = 0; i < TargetsLength; i++)
	//{
	//	APlayableCharacter* PC = Cast<APlayableCharacter>(NewTargets[i]);
	//	if (PC)
	//	{
	//		SetSensedTarget(PC, PC->GetActorLocation());
	//	}
	//	else
	//	{
	//		SetSensedTarget(NULL, NewTargets[i]->GetActorLocation());
	//	}
	//}
}

void ABotAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	Super::UpdateControlRotation(DeltaTime, bUpdatePawn);
	//Look toward focus
	/*FVector FocalPoint = GetFocalPoint();
	if (!FocalPoint.IsZero() && GetPawn())
	{
		FVector Direction = FocalPoint - GetPawn()->GetActorLocation();
		FRotator NewControlRotation = Direction.Rotation();

		NewControlRotation.Yaw = FRotator::ClampAxis(NewControlRotation.Yaw);

		SetControlRotation(NewControlRotation);

		APawn* const MyPawn = GetPawn();
		if (MyPawn && bUpdatePawn)
		{
			MyPawn->FaceRotation(NewControlRotation, DeltaTime);
		}
	}*/
}

void ABotAIController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	if (BehaviorComp)
	{
		BehaviorComp->StopTree();
	}

	StopMovement();

	SetEnemy(NULL);

	ABotCharacter* MyBot = Cast<ABotCharacter>(GetPawn());
	AWeaponMaster* MyWeapon = MyBot ? MyBot->GetWeapon() : NULL;
	if (MyWeapon == NULL)
	{
		return;
	}
	MyBot->StopShooting();
}

void ABotAIController::BeginInactiveState()
{
	Super::BeginInactiveState();

	if (bCanRespawn == false)
	{
		return;
	}

	AGameStateBase const* GameState = GetWorld()->GetGameState();
	const float MinRespawnDelay = 1.0f;
	GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &ABotAIController::Respawn, MinRespawnDelay);
}

void ABotAIController::Respawn()
{
	GetWorld()->GetAuthGameMode()->RestartPlayer(this);
}

void ABotAIController::FocusOnWaypoint(FVector Waypoint)
{
	APawn* MyBot = GetPawn();
	const FVector FocalPoint = GetFocalPoint();
	if (MyBot && !FocalPoint.IsZero())
	{
		const FVector InitialDirection = FocalPoint - MyBot->GetActorLocation();
		const FRotator InitialRotation = InitialDirection.Rotation();
		//InitialRotation.Yaw = FRotator::ClampAxis(InitialRotation.Yaw);

		FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(InitialDirection, Waypoint);
		FRotator NewRotation = FMath::LerpRange(InitialRotation, TargetRotation, 15.0f);

		SetControlRotation(TargetRotation);
		MyBot->FaceRotation(TargetRotation);
	}
}

void ABotAIController::FindSourceOfSightStimulus()
{
	const AActor* MySeenTarget = GetSeenTarget();
	if (PerceptionComp && BlackboardComp)
	{
		if (PerceptionComp->HasAnyCurrentStimulus(*MySeenTarget) && PerceptionComp->GetYoungestStimulusAge(*MySeenTarget) > 5.0f)
		{
			BlackboardComp->SetValue<UBlackboardKeyType_Vector>(SeenStimuliKeyID, LastSeenLocation);
		}
	}
}

void ABotAIController::FindClosestEnemy()
{
	APawn* MyBot = GetPawn();
	if (MyBot == NULL)
	{
		return;
	}

	const FVector MyLoc = MyBot->GetActorLocation();
	float BestDistSq = MAX_FLT;
	APlayableCharacter* BestPawn = NULL;

	for (APlayableCharacter* Playable : TActorRange<APlayableCharacter>(GetWorld()))
	{
		APawn* PlayablePawn = Cast<APawn>(Playable);
		if (Playable->IsAlive() && GetTeamAttitudeTowards(*PlayablePawn->GetController()) == ETeamAttitude::Hostile)
		{
			const float DistSq = (Playable->GetActorLocation() - MyLoc).SizeSquared();
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestPawn = Playable;
			}
		}
	}

	if (BestPawn)
	{
		SetEnemy(BestPawn);
	}
}

bool ABotAIController::FindClosestEnemyWithLOS(APlayableCharacter* ExcludeEnemy)
{
	bool bGotEnemy = false;
	APawn* MyBot = GetPawn();
	if (MyBot != NULL)
	{
		const FVector MyLoc = MyBot->GetActorLocation();
		float BestDistSq = MAX_FLT;
		APlayableCharacter* BestPawn = NULL;

		for (APlayableCharacter* Playable : TActorRange<APlayableCharacter>(GetWorld()))
		{
			//TestPawn->IsEnemyFor(this)
			APawn* PlayablePawn = Cast<APawn>(Playable);
			if (Playable != ExcludeEnemy && Playable->IsAlive() && GetTeamAttitudeTowards(*PlayablePawn->GetController()) == ETeamAttitude::Hostile)
			{
				if (HasWeaponLOSToEnemy(Playable, true) == true)
				{
					const float DistSq = (Playable->GetActorLocation() - MyLoc).SizeSquared();
					if (DistSq < BestDistSq)
					{
						BestDistSq = DistSq;
						BestPawn = Playable;
					}
				}
			}
		}
		if (BestPawn)
		{
			SetEnemy(BestPawn);
			bGotEnemy = true;
		}
	}
	return bGotEnemy;
}

bool ABotAIController::HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const
{
	ABotCharacter* MyBot = Cast<ABotCharacter>(GetPawn());

	bool bHasLOS = false;

	//Trace to determine LOS
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(AIWeaponLosTrace), true, GetPawn());
	TraceParams.bReturnPhysicalMaterial = true;
	FVector StartPoint = MyBot->GetActorLocation();
	const FVector EndPoint = InEnemyActor->GetActorLocation();

	StartPoint.Z += GetPawn()->BaseEyeHeight;

	FHitResult Hit(ForceInit);
	
	GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, COLLISION_WEAPON, TraceParams);
	if (Hit.bBlockingHit == true)
	{
		// Blocking hit, check if it's the enemy
		AActor* HitActor = Hit.GetActor();
		if (Hit.GetActor() != NULL)
		{
			// Our actor is the enemy
			if (HitActor == InEnemyActor)
			{
				bHasLOS = true;
			}
			// Our actor may be an enemy, check affiliation
			else if (bAnyEnemy == true)
			{
				//ACharacter* HitChar = Cast<ACharacter>(HitActor);
				APawn* PlayablePawn = Cast<APawn>(HitActor);
				if (PlayablePawn != NULL)
				{
					if (GetTeamAttitudeTowards(*PlayablePawn->GetController()) == ETeamAttitude::Hostile)
					{
						bHasLOS = true;
					}
					/*APlayerState* HitPlayerState = Cast<APlayerState>(HitChar->GetPlayerState());
					APlayerState* MyPlayerState = Cast<APlayerState>(PlayerState);
					if ((HitPlayerState != NULL) && (MyPlayerState != NULL) && HitPlayerState != MyPlayerState)
					{
						bHasLOS = true;
					}*/
					//Need more info to determine if enemy
				}
			}
		}
	}
	return bHasLOS;
}

void ABotAIController::ShootEnemy()
{
	ABotCharacter* MyBot = Cast<ABotCharacter>(GetPawn());
	AWeaponMaster* MyWeapon = MyBot ? MyBot->GetWeapon() : NULL;
	if (MyWeapon == NULL)
	{
		return;
	}

	bool bCanShoot = false;
	APlayableCharacter* Enemy = GetEnemy();
	if (Enemy && (Enemy->IsAlive() && MyWeapon->GetCurrentAmmo() > 0) && (MyWeapon->CanFire() == true))
	{
		if (LineOfSightTo(Enemy, MyBot->GetActorLocation()))
		{
			bCanShoot = true;
		}
	}

	if (bCanShoot)
	{
		MyBot->StartShooting();
	}
	else
	{
		MyBot->StopShooting();
	}
}

void ABotAIController::CheckAmmo(const class AWeaponMaster* CurrentWeapon)
{
	if (CurrentWeapon && BlackboardComp)
	{
		const int32 Ammo = CurrentWeapon->GetCurrentAmmo();
		const int32 MaxAmmo = CurrentWeapon->GetMaxAmmo();
		const float Ratio = (float) Ammo / (float) MaxAmmo;

		BlackboardComp->SetValue<UBlackboardKeyType_Bool>(NeedAmmoKeyID, (Ratio <= 0.1f));
	}
}

void ABotAIController::SetEnemy(class APawn* InPawn)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValue<UBlackboardKeyType_Object>(EnemyKeyID, InPawn);
		SetFocus(InPawn);
	}
}

void ABotAIController::SetSeenTarget(AActor* NewTarget, FVector& TargetLocation)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValue<UBlackboardKeyType_Vector>(SeenStimuliKeyID, TargetLocation);
		BlackboardComp->SetValue<UBlackboardKeyType_Object>(SeenActorKeyID, NewTarget);
		DrawDebugSphere(GetWorld(), TargetLocation, 20.0f, 12, FColor::Red, false, 2.0f);
	}
}

void ABotAIController::SetHeardTarget(AActor* NewTarget, const FVector& Location)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValue<UBlackboardKeyType_Vector>(HearingStimuliKeyID, Location);
	}
}

APlayableCharacter* ABotAIController::GetEnemy() const
{
	if (BlackboardComp)
	{
		return Cast<APlayableCharacter>(BlackboardComp->GetValue<UBlackboardKeyType_Object>(EnemyKeyID));
	}

	return NULL;
}

AActor* ABotAIController::GetSeenTarget() const
{
	if (BlackboardComp)
	{
		return Cast<AActor>(BlackboardComp->GetValue<UBlackboardKeyType_Object>(SeenActorKeyID));
	}

	return NULL;
}