// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasLOSTo.generated.h"

/**
 * Check if pawn has Line of Sight to a specified object
 */
UCLASS()
class THESISGAME_API UBTDecorator_HasLOSTo : public UBTDecorator
{
	GENERATED_UCLASS_BODY()
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
	UPROPERTY(EditAnywhere, Category = Condition)
	struct FBlackboardKeySelector EnemyKey;

private:
	bool LOSTrace(AActor* InActor, AActor* InEnemyActor, const FVector& EndLocation) const;
};
