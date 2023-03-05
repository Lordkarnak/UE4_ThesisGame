// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_HasLOSTo.h"
#include "ThesisGame.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BotCharacter.h"
#include "BotAIController.h"

UBTDecorator_HasLOSTo::UBTDecorator_HasLOSTo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Has LOS To";
	EnemyKey.AddObjectFilter(this, *NodeName, AActor::StaticClass());
	EnemyKey.AddVectorFilter(this, *NodeName);
}

bool UBTDecorator_HasLOSTo::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	AAIController* MyController = OwnerComp.GetAIOwner();
	bool HasLOS = false;

	if (MyController && MyBlackboard)
	{
		auto MyID = MyBlackboard->GetKeyID(EnemyKey.SelectedKeyName);
		auto TargetKeyType = MyBlackboard->GetKeyType(MyID);

		FVector TargetLocation;
		bool bGotTarget = false;
		AActor* EnemyActor = NULL;
		if (TargetKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(MyID);
			EnemyActor = Cast<AActor>(KeyValue);
			if (EnemyActor)
			{
				TargetLocation = EnemyActor->GetActorLocation();
				bGotTarget = true;
			}
		}
		else if (TargetKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			TargetLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(MyID);
			bGotTarget = true;
		}
		if (bGotTarget)
		{
			if (LOSTrace(OwnerComp.GetOwner(), EnemyActor, TargetLocation) == true)
			{
				HasLOS = true;
			}
		}
	}

	return HasLOS;
}

bool UBTDecorator_HasLOSTo::LOSTrace(AActor* InActor, AActor* InEnemyActor, const FVector& EndPoint) const
{
	ABotAIController* MyController = Cast<ABotAIController>(InActor);
	ABotCharacter* MyBot = MyController ? Cast<ABotCharacter>(MyController->GetPawn()) : NULL;

	bool HasLOS = false;

	if (MyBot != NULL)
	{
		FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(AILosTrace), true, InActor);
		TraceParams.bReturnPhysicalMaterial = true;
		TraceParams.AddIgnoredActor(MyBot);

		const FVector StartPoint = MyBot->GetActorLocation();
		FHitResult Hit(ForceInit);
		GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, COLLISION_WEAPON, TraceParams);
		if (Hit.bBlockingHit == true)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor != NULL)
			{
				//If we hit the actor we were looking for, we have LOS
				if (HitActor == InActor)
				{
					HasLOS = true;
				}
				// if we hit an actor that is controlled by the player, consider it LOS
				else
				{
					APlayableCharacter* HitChar = Cast<APlayableCharacter>(HitActor);
					if ((HitChar != NULL) && HitChar->IsPlayerControlled())
					{
						HasLOS = true;
					}
				}
			}
			else
			{
				if (InEnemyActor == NULL)
				{
					// Check distance between what we hit and the target. We may hit the target
					FVector HitDelta = Hit.ImpactPoint - StartPoint;
					FVector TargetDelta = EndPoint - StartPoint;
					HasLOS = (bool) (TargetDelta.SizeSquared() < HitDelta.SizeSquared());
				}
			}
		}
	}

	return HasLOS;
}