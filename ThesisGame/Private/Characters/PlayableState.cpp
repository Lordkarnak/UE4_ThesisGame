// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayableState.h"


int32 APlayableState::GetNumKills() const
{
	return Kills;
}

int32 APlayableState::GetNumDeaths() const
{
	return Deaths;
}

int32 APlayableState::GetNumQuestsTaken() const
{
	return QuestsTaken;
}

void APlayableState::AddKill()
{
	Kills++;
}

void APlayableState::AddDeath()
{
	Deaths++;
}

void APlayableState::AddQuestTaken()
{
	QuestsTaken++;
}