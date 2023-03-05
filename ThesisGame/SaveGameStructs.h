// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Quests/Quest.h"
#include "Characters/PlayableCharacter.h"
#include "Weapons/WeaponMaster.h"
#include "SaveGameStructs.generated.h"

USTRUCT()
struct FQuestData
{
    GENERATED_BODY()
    
    UPROPERTY()
    TSubclassOf<class AQuest> QuestClass;

    UPROPERTY()
    int32 CurrentStage;

    UPROPERTY()
    bool IsActive;

public:
    FQuestData()
    {
        QuestClass = NULL;
        CurrentStage = 0;
        IsActive = false;
    }

    FQuestData(AQuest* Quest)
    {
        if (Quest != nullptr)
        {
            QuestClass = Quest->GetClass();
            CurrentStage = Quest->GetCurrentStage();
            IsActive = Quest->IsActive();
        }
        else
        {
            QuestClass = NULL;
            CurrentStage = 0;
            IsActive = false;
        }
    }

    void ToString()
    {
        UE_LOG(LogTemp, Display, TEXT("Quest Class: %s \n Current Stage: %s \n Quest is active? %s"), *QuestClass->GetName(), CurrentStage, *((IsActive) ? "true" : "false"));
    }
};

USTRUCT()
struct FInventoryData
{
    GENERATED_BODY()

    UPROPERTY()
    TSubclassOf<class AWeaponMaster> WeaponClass;

    UPROPERTY()
    int32 CurrentAmmo;

    UPROPERTY()
    int32 MaxAmmo;

public:
    FInventoryData()
    {
        WeaponClass = NULL;
        CurrentAmmo = 0;
        MaxAmmo = 0;
    }

    FInventoryData(AWeaponMaster* Weapon)
    {
        if (Weapon != nullptr)
        {
            WeaponClass = Weapon->GetClass();
            CurrentAmmo = Weapon->GetCurrentAmmo();
            MaxAmmo = Weapon->GetMaxAmmo();
        }
        else
        {
            WeaponClass = NULL;
            CurrentAmmo = 0;
            MaxAmmo = 0;
        }
    }

    void ToString()
    {
        UE_LOG(LogTemp, Display, TEXT("Weapon Class: %s\n Current ammo: %s \n Max ammo: %s"), *WeaponClass->GetName(), CurrentAmmo, MaxAmmo);
    }
};

USTRUCT()
struct FActorData
{
    GENERATED_BODY()
    
    UPROPERTY()
    int32 ActorID;
    
    UPROPERTY()
    float CurrentHealth;

    UPROPERTY()
    float BaseHealth;

    UPROPERTY()
    FInventoryData LastEquippedWeapon;

    UPROPERTY()
    TArray<FInventoryData> LastInventory;

    UPROPERTY()
    FTransform LastPosition;
    
    UPROPERTY()
    bool PositionChanged;

public:
    FActorData()
    {
        ActorID = 0;
        CurrentHealth = 0;
        BaseHealth = 0;
        LastEquippedWeapon = NULL;
        LastInventory = TArray<FInventoryData>();
        LastPosition = FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector(1.0));
        PositionChanged = false;
    }

    FActorData(APlayableCharacter* TargetActor)
    {
        if (TargetActor != nullptr)
        {
            ActorID = TargetActor->GetID();
            CurrentHealth = TargetActor->GetHealth();
            BaseHealth = TargetActor->GetBaseHealth();
            AWeaponMaster* CurrentWeapon = TargetActor->GetCurrentWeapon();
            if (CurrentWeapon)
            {
                LastEquippedWeapon = FInventoryData(CurrentWeapon);
            }
            TArray<class AWeaponMaster*> Inventory = TargetActor->GetCurrentInventory();
            int32 InventoryCount = Inventory.Num();
            if (InventoryCount > 0)
            {
                for (int32 i = 0; i < InventoryCount; i++)
                {
                    if (Inventory[i])
                    {
                        const FInventoryData InvData = FInventoryData(Inventory[i]);
                        LastInventory.Add(InvData);
                    }
                }
            }
            LastPosition = TargetActor->GetActorTransform();
            PositionChanged = true;
        }
        else
        {
            ActorID = 0;
            CurrentHealth = 0;
            BaseHealth = 0;
            LastEquippedWeapon = NULL;
            LastInventory = TArray<FInventoryData>{ NULL };
            LastPosition = FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector(1.0));
            PositionChanged = false;
        }
    }

    void ToString()
    {
        UE_LOG(LogTemp, Display, TEXT("ActorID: %s \n CurrentHealth: %s \n BaseHealth: %s \n Last Equipped Weapon: \n"), ActorID, CurrentHealth, BaseHealth);
        LastEquippedWeapon.ToString();
        UE_LOG(LogTemp, Display, TEXT("Last Inventory: "));
        if (LastInventory.Num() > 0)
        {
            for (int32 i = 0; i < LastInventory.Num(); i++)
            {
                LastInventory[i].ToString();
            }
        }
        LastPosition.DebugPrint();
        UE_LOG(LogTemp, Display, TEXT("Position changed: %s"), *((PositionChanged) ? "true" : "false"));
    }
};