// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestTrigger.h"
#include "Characters/PlayableCharacter.h"
#include "ThesisGameGameMode.h"
#include "Public/ThesisGameGameState.h"
//#include "Kismet/GameplayStatics.h"

AQuestTrigger::AQuestTrigger(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	OnActorBeginOverlap.AddDynamic(this, &AQuestTrigger::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &AQuestTrigger::OnOverlapEnd);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AQuestTrigger::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AQuestTrigger::BeginPlay()
{
	Super::BeginPlay();

	//DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Purple, true, -1, 0, 5);
	FTimerHandle QuestFillTimer;
	GetWorldTimerManager().SetTimer(QuestFillTimer, this, &AQuestTrigger::RegisterTargetQuest, 0.3f, false);
}

void AQuestTrigger::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
	bool IsTriggeredBySetActor = (TriggeredBy != nullptr && TriggeredBy == OtherActor);
	bool IsTriggeredByPlayer = Cast<APlayableCharacter>(OtherActor) != nullptr;
	UE_LOG(LogTemp, Warning, TEXT("(%s) overlapped with (%s)."), *OverlappedActor->GetName(), *OtherActor->GetName());
	if (IsTriggeredBySetActor || IsTriggeredByPlayer)
	{
		if (TargetQuest == nullptr)
		{
			RegisterTargetQuest();
		}
		if (TargetQuest != nullptr && TargetQuest->Update(TargetStage))
		{
			SetLifeSpan(1.0f);
			//FlushPersistentDebugLines(GetWorld());
			//Destroy();
		}
	}
}

void AQuestTrigger::OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{

}

void AQuestTrigger::RegisterTargetQuest()
{
	if (TargetQuestClass != NULL && TargetQuest == nullptr)
	{
		UWorld* MyWorld = GetWorld();
		if (MyWorld)
		{
			AThesisGameGameState* MyGameState = MyWorld->GetGameState<AThesisGameGameState>();
			if (MyGameState)
			{
				TargetQuest = MyGameState->FindQuest(TargetQuestClass);
				if (TargetQuest)
				{
					UE_LOG(LogTemp, Warning, TEXT("Target quest of type(%s) found."), *TargetQuest->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No quest of type(%s) found."), *TargetQuestClass->GetName());
				}
			}
		}
	}

	// Find target quest using the class specified in defaults
	//if (TargetQuest == nullptr)
	//{
	//	TArray<AActor*> FoundQuests;
	//	UGameplayStatics::GetAllActorsOfClass(MyWorld, TargetQuestClass, FoundQuests);
	//	if (FoundQuests.Num() > 0) // Should be only one
	//	{
	//		TargetQuest = Cast<AQuest>(FoundQuests[0]);
	//		FString DebugTarget = FoundQuests[0]->GetClass()->GetFName().ToString();
	//		UE_LOG(LogTemp, Warning, TEXT("Target quest of type(%s) found."), *DebugTarget);
	//	}
	//}
}