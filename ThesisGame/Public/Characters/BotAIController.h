// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BotAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
struct FAIStimulus;
/**
 * 
 */
UCLASS()
class THESISGAME_API ABotAIController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(class APawn* InPawn) override;

	virtual void OnUnPossess() override;

	UFUNCTION(BlueprintCallable, Category = "Perception|AI")
	void OnTargetSenseUpdated(AActor* TargetActor, FAIStimulus Stimulus);

	/** Our blackboard reference */
	UPROPERTY(Transient, BlueprintReadOnly)
	UBlackboardComponent* BlackboardComp;

	/** Our behavior reference */
	UPROPERTY(Transient, BlueprintReadOnly)
	UBehaviorTreeComponent* BehaviorComp;

	/** Our perception component */
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	UAIPerceptionComponent* PerceptionComp;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	UAISenseConfig_Hearing* HearingConfig;

public:
	ABotAIController();

	/** Overrides */
	//virtual void BeginPlay() override;

	/** IGenericTeamAgentInterface - Get affiliation based on a team number. */
	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	
	/** IGenericTeamAgentInterface - Set the team id on this controller */
	void SetGenericTeamId(const FGenericTeamId& InTeamID) override;

	/**IGenericTeamAgentInterface - Get the team id for this controller */
	FGenericTeamId GetGenericTeamId() const override;

	/** Update direction AI is looking based on FocalPoint */
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;

	// Controller Interface
	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;
	virtual void BeginInactiveState() override;
	// End Controller Interface
	
	/** End overrides */

	void Respawn();

	UFUNCTION(BlueprintCallable, Category = Behavior)
	void FocusOnWaypoint(FVector Waypoint);

	UFUNCTION(BlueprintCallable, Category = Behavior)
	void FindSourceOfSightStimulus();

	UFUNCTION(BlueprintCallable, Category = Behavior)
	void FindClosestEnemy();

	UFUNCTION(BlueprintCallable, Category = Behavior)
	bool FindClosestEnemyWithLOS(APlayableCharacter* ExcludeEnemy);

	bool HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const;

	UFUNCTION(BlueprintCallable, Category = Behavior)
	void ShootEnemy();

	void CheckAmmo(const class AWeaponMaster* CurrentWeapon);

	void SetEnemy(class APawn* TargetPawn);

	void SetSeenTarget(AActor* NewPawn, FVector& TargetLocation);

	void SetHeardTarget(AActor* NewPawn, const FVector& Location);

	class APlayableCharacter* GetEnemy() const;

	AActor* GetSeenTarget() const;

	UPROPERTY(EditAnywhere, Category = "AI Properties")
	uint8 bCanRespawn : 1;

protected:
	bool LOSTrace(APlayableCharacter* InEnemyChar) const;

	int32 EnemyKeyID;
	int32 NeedAmmoKeyID;
	int32 SeenStimuliKeyID;
	int32 SeenActorKeyID;
	int32 HearingStimuliKeyID;

	FTimerHandle TimerHandle_Respawn;

	FVector LastSeenLocation;

private:
	FGenericTeamId AITeamId;

public:
	/** Returns BlackboardComp subobject **/
	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }
};
