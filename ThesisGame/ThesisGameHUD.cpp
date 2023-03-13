// Copyright Epic Games, Inc. All Rights Reserved.

#include "ThesisGameHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "Characters/PlayableCharacter.h"
#include "Characters/PlayableController.h"
#include "Weapons/WeaponMaster.h"
#include "Pickups/Pickup.h"
#include "Pickups/SaveTerminal.h"
#include "Quests/Quest.h"
#include "UObject/ConstructorHelpers.h"

AThesisGameHUD::AThesisGameHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;
	HUDWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	//HUDWidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AThesisGameHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	


	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	int32 AimOffsetX = -10.0f;
	int32 AimOffsetY = 10.0f;
	APlayableController* MyPCOwner = Cast<APlayableController>(PlayerOwner);
	if (MyPCOwner)
	{
		// find center of the Canvas
		//const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
		const FVector2D Center((Canvas->ClipX / 2), (Canvas->ClipY / 2));

		APlayableCharacter* MyPawn = Cast<APlayableCharacter>(MyPCOwner->GetPawn());
		if (MyPawn && MyPawn->IsTargeting())
		{
			AimOffsetX = FMath::FInterpTo(AimOffsetX, 0.0f, GetWorld()->DeltaTimeSeconds, 0.1f);
		}

		const FVector2D CrosshairDrawPosition((Center.X + AimOffsetX), (Center.Y + AimOffsetY));

		// draw the crosshair
		FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
	}
}

void AThesisGameHUD::EnableInteractionWidget(USceneComponent* ParentComp)
{
	// Cleanup
	if (HUDInteractionWidget != nullptr)
	{
		HUDInteractionWidget->RemoveFromViewport();
		HUDInteractionWidget = nullptr;
	}

	// Create as new
	AThesisGameGameMode* MyGameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
	if (MyGameMode && MyGameMode->InteractionWidget != NULL)
	{
		// Create the widget
		HUDInteractionWidget = CreateWidget<UUserWidget>(GetWorld(), MyGameMode->InteractionWidget);
		HUDInteractionWidget->AddToViewport();

		//FActorSpawnParameters SpawnParams;
		//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		//FTransform SpawnTransform(GetActorRotation(), ParentComp->GetComponentLocation());
		//MyWidgetComponent = GetWorld()->SpawnActorDeferred<UWidgetComponent>(UWidgetComponent::StaticClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		//if (HUDWidgetComponent)
		//{
		//	// Parametize the component
		//	if (ParentComp)
		//	{
		//		FActorSpawnParameters SpawnInfo;
		//		//SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		//		//HUDWidgetComponent = ParentComp->GetWorld()->SpawnActor<UWidgetComponent>(SpawnInfo);
		//		if (HUDWidgetComponent)
		//		{
		//			//FAttachmentTransformRules HUDAttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepRelative, false);
		//			//HUDWidgetComponent->SetWidgetClass(MyGameMode->InteractionWidget);
		//			bool AttachAttempt = HUDWidgetComponent->AttachToComponent(ParentComp, FAttachmentTransformRules::KeepRelativeTransform);

		//			HUDWidgetComponent->InitWidget();
		//			HUDWidgetComponent->SetHiddenInGame(false);
		//			HUDWidgetComponent->SetDrawAtDesiredSize(true);
		//			HUDWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
		//			HUDWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//			
		//			HUDWidgetComponent->SetRelativeLocation(FVector(0.0f, 25.0f, 0.0f));
		//			HUDWidgetComponent->SetWidget(HUDInteractionWidget);
		//			HUDWidgetComponent->UpdateWidget();

		//			UE_LOG(LogTemp, Display, TEXT("Was attachment successful? %s"), *FString((AttachAttempt) ? "true" : "false"));
		//			//HUDWidgetComponent->AttachToComponent(ParentComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		//		}
		//	}
		//	//MyInteractionWidget->AttachToComponent(OverlappedComponent, FAttachmentTransformRules::KeepRelativeTransform);
		//}
	}
}

void AThesisGameHUD::DisableInteractionWidget()
{
	// Only cleanup
	if (HUDWidgetComponent != nullptr)
	{
		//MyWidgetComponent->SetWidget(NULL);
		//MyWidgetComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		//MyWidgetComponent->DestroyComponent();
		//MyWidgetComponent = nullptr;
	}
	if (HUDInteractionWidget != nullptr)
	{
		HUDInteractionWidget->RemoveFromViewport();
		HUDInteractionWidget = nullptr;
	}
}

void AThesisGameHUD::EnableQuestLogWidget()
{
	AThesisGameGameMode* MyGameMode = GetWorld()->GetAuthGameMode<AThesisGameGameMode>();
	if (MyGameMode)
	{
		// Cleanup
		if (HUDQuestLogWidget != nullptr)
		{
			HUDQuestLogWidget->RemoveFromViewport();
			HUDQuestLogWidget = nullptr;
		}
		
		HUDQuestLogWidget = CreateWidget<UUserWidget>(GetWorld(), MyGameMode->LogWidgetClass);
		HUDQuestLogWidget->AddToViewport();

		// Control player controller properties, needed here as blueprints access this function as well
		APlayerController* MyPlayerController = GetOwningPlayerController();
		if (MyPlayerController != nullptr)
		{
			FInputModeGameAndUI MenuMode;
			MenuMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			MyPlayerController->SetInputMode(MenuMode);
			MyPlayerController->bShowMouseCursor = true;
			MyPlayerController->SetPause(true);
			MyPlayerController->SetCinematicMode(true, false, false, true, true);
		}
	}
}

void AThesisGameHUD::DisableQuestLogWidget()
{
	if (HUDQuestLogWidget != nullptr)
	{
		HUDQuestLogWidget->RemoveFromViewport();
		HUDQuestLogWidget = nullptr;

		// Control player controller properties, needed here as blueprints access this function as well
		APlayerController* MyPlayerController = GetOwningPlayerController();
		if (MyPlayerController != nullptr)
		{
			MyPlayerController->SetPause(false);
			FInputModeGameOnly Mode;
			Mode.SetConsumeCaptureMouseDown(false);
			MyPlayerController->SetInputMode(Mode);
			MyPlayerController->bShowMouseCursor = false;
			MyPlayerController->SetCinematicMode(false, false, false, true, true);
		}
	}
}

void AThesisGameHUD::ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (CurrentMenuWidget != nullptr)
	{
		CurrentMenuWidget->RemoveFromViewport();
		CurrentMenuWidget = nullptr;
	}
	if (NewWidgetClass != nullptr)
	{
		CurrentMenuWidget = CreateWidget<UUserWidget>(GetWorld(), NewWidgetClass);
		if (CurrentMenuWidget != nullptr)
		{
			CurrentMenuWidget->AddToViewport();
		}
	}
}

void AThesisGameHUD::DetermineInteraction(UPrimitiveComponent* TargetComp, AActor* TargetActor)
{
	AWeaponMaster* WeaponType = nullptr;
	APickup* PickupType = nullptr;
	ASaveTerminal* Terminal = nullptr;
	if (TargetComp != nullptr)
	{
		WeaponType = Cast<AWeaponMaster>(TargetComp);
		PickupType = Cast<APickup>(TargetComp);
		Terminal = Cast<ASaveTerminal>(TargetComp);
	}
	else if (TargetActor != nullptr)
	{
		WeaponType = Cast<AWeaponMaster>(TargetActor);
		PickupType = Cast<APickup>(TargetActor);
		Terminal = Cast<ASaveTerminal>(TargetActor);
	}
	
	if (Terminal != nullptr)
	{
		CurrentInteraction = EInteractionTypes::Save;
	}
	else if (WeaponType != nullptr || PickupType != nullptr)
	{
		CurrentInteraction = EInteractionTypes::Take;
	}
	else
	{
		CurrentInteraction = EInteractionTypes::Activate;
	}
	//SCurrentInteraction = UEnum::GetValueAsString<EInteractionTypes::Type>(CurrentInteraction);
	UEnum::GetDisplayValueAsText(CurrentInteraction, CurrentInteractionText);
}

EInteractionTypes::Type AThesisGameHUD::GetCurrentInteractionEnum() const
{
	return CurrentInteraction;
}

FText AThesisGameHUD::GetCurrentInteractionText() const
{
	return CurrentInteractionText;
}

void AThesisGameHUD::SetInMenuActiveQuest(AQuest* NewQuest)
{
	InMenuQuest = NewQuest;
}

AQuest* AThesisGameHUD::GetInMenuActiveQuest() const
{
	return InMenuQuest;
}

void AThesisGameHUD::ShowMSGWidget(TSubclassOf<class UUserWidget> WidgetClass)
{
	if (WidgetClass != NULL)
	{
		/*if (MSGWidget != nullptr)
		{
			MSGWidget->RemoveFromViewport();
			MSGWidget = nullptr;
		}*/
		//static ConstructorHelpers::FClassFinder<UUserWidget> StartQuestMessageObj(TEXT("/Game/ThesisGame/Menus/MSG_QuestStarted"));
		UUserWidget* MSG = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
		MSG->AddToViewport();
		FTimerHandle MSGHideTimer;
		FTimerDelegate DeleteMessageDelegate;
		DeleteMessageDelegate.BindUFunction(this, FName("MarkWidgetForDeletion"), MSG);
		GetWorldTimerManager().SetTimer(MSGHideTimer, DeleteMessageDelegate, 5.0f, false);
	}
}

void AThesisGameHUD::AddQuestToQueue(AQuest* WhichQuest)
{
	QuestsQueue.AddUnique(WhichQuest);
	FTimerHandle RemoveTimerHandle;
	FTimerDelegate RemoveFromQueueDelegate;
	RemoveFromQueueDelegate.BindUFunction(this, FName("RemoveFromQuestQueue"), WhichQuest);
	// Get the current queue and add 5 seconds for every quest in queue, ensuring that all quests will be displayed before cleaning the queue
	int32 NumQuests = FMath::Max(1, QuestsQueue.Num());
	float DeleteDelay = (5.0f * NumQuests);
	GetWorldTimerManager().SetTimer(RemoveTimerHandle, RemoveFromQueueDelegate, DeleteDelay, false);
}

void AThesisGameHUD::AddObjectiveToQueue(const FQuestObjective& Objective)
{
	ObjectivesQueue.AddUnique(Objective);
	FTimerHandle RemoveTimerHandle;
	FTimerDelegate RemoveFromQueueDelegate;
	RemoveFromQueueDelegate.BindUFunction(this, FName("RemoveFromObjectiveQueue"), Objective);
	// Get the current queue and add 5 seconds for every objective in queue, ensuring that all objectives will be displayed before cleaning the queue
	int32 NumObjectives = FMath::Max(1, ObjectivesQueue.Num());
	float DeleteDelay = (5.0f * NumObjectives);
	GetWorldTimerManager().SetTimer(RemoveTimerHandle, RemoveFromQueueDelegate, DeleteDelay, false);
}

TArray<AQuest*> AThesisGameHUD::GetQuestsQueue() const
{
	return QuestsQueue;
}

TArray<FQuestObjective> AThesisGameHUD::GetObjectivesQueue() const
{
	return ObjectivesQueue;
}

void AThesisGameHUD::RemoveFromQuestQueue(AQuest* WhichQuest)
{
	QuestsQueue.Remove(WhichQuest);
}

void AThesisGameHUD::RemoveFromObjectiveQueue(const FQuestObjective& Objective)
{
	ObjectivesQueue.Remove(Objective);
}

void AThesisGameHUD::MarkWidgetForDeletion(UUserWidget* WhichWidget)
{
	if (WhichWidget != nullptr)
	{
		WhichWidget->RemoveFromViewport();
		WhichWidget->Destruct();
	}
}