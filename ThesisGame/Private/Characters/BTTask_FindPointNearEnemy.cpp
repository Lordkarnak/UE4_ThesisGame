// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindPointNearEnemy.h"
#include "BotAIController.h"
#include "PlayableCharacter.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "NavigationSystem.h"

UBTTask_FindPointNearEnemy::UBTTask_FindPointNearEnemy(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

EBTNodeResult::Type UBTTask_FindPointNearEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABotAIController* MyController = Cast<ABotAIController>(OwnerComp.GetAIOwner());
	if (MyController == NULL)
	{
		return EBTNodeResult::Failed;
	}

	APawn* MyBot = MyController->GetPawn();
	APlayableCharacter* Enemy = MyController->GetEnemy();
	if (Enemy && MyBot && Enemy->IsAlive())
	{
		//const float SearchRadius = 200.0f;
		const FVector SearchOrigin = Enemy->GetActorLocation() + DistanceFromEnemy * (MyBot->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
		FVector Loc(0);
		UNavigationSystemV1::K2_GetRandomReachablePointInRadius(MyController, SearchOrigin, Loc, SearchRadius);
		if (Loc != FVector::ZeroVector)
		{
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), Loc);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}