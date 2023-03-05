// Fill out your copyright notice in the Description page of Project Settings.


#include "ThesisGameGameState.h"
#include "Quests/Quest.h"

AThesisGameGameState::AThesisGameGameState()
{

}

bool AThesisGameGameState::AddQuest(AQuest* NewQuest)
{
	if (QuestList.Contains<FName>(NewQuest->QuestName))
	{
		return false;
	}

	CurrentQuestList.AddUnique(NewQuest);
	QuestList.AddUnique(NewQuest->QuestName);
	if (NewQuest->bAutoActivate)
	{
		ActiveQuest = NewQuest;
	}
	return true;
}

bool AThesisGameGameState::RemoveQuest(AQuest* WhichQuest, bool bForceRemove)
{
	if (QuestList.Contains<FName>(WhichQuest->QuestName))
	{
		if (WhichQuest->IsCompleted())
		{
			CurrentQuestList.Remove(WhichQuest);
			CompletedQuestList.AddUnique(WhichQuest);
			return true;
		}
		else if (!WhichQuest->IsCompleted() && bForceRemove)
		{
			WhichQuest->Complete();
			CurrentQuestList.Remove(WhichQuest);
			CompletedQuestList.AddUnique(WhichQuest);
			return true;
		}
	}

	return false;
}

AQuest* AThesisGameGameState::GetActiveQuest() const
{
	return ActiveQuest;
}

void AThesisGameGameState::SetActiveQuest(AQuest* WhichQuest)
{
	if (WhichQuest != ActiveQuest && QuestList.Contains(WhichQuest->QuestName))
	{
		ActiveQuest = WhichQuest;
	}
}

TArray<AQuest*> AThesisGameGameState::GetCurrentQuests() const
{
	return CurrentQuestList;
}

TArray<AQuest*> AThesisGameGameState::GetCompletedQuests() const
{
	return CompletedQuestList;
}

AQuest* AThesisGameGameState::FindQuest(class TSubclassOf<AQuest> WhichQuestClass)
{
	int32 NumQuests = CurrentQuestList.Num();
	for (int32 i = 0; i < NumQuests; i++)
	{
		if (CurrentQuestList[i]->GetClass() == WhichQuestClass)
		{
			return CurrentQuestList[i];
		}
	}

	return nullptr;
}