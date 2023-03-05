// Fill out your copyright notice in the Description page of Project Settings.


#include "Objective.h"

AObjective::AObjective()
{
	bIsRunning = false;
	bIsCompleted = false;
	bIsFailed = false;
}

void AObjective::Start()
{
	if (!bIsRunning)
	{
		bIsRunning = true;
	}
}

void AObjective::Finish(bool bFailed)
{
	if (bIsRunning)
	{
		bIsRunning = false;
		bIsCompleted = true;
		bIsFailed = bFailed;
	}
}

bool AObjective::IsActive()
{
	return bIsRunning && !bIsCompleted && !bIsFailed;
}

bool AObjective::IsCompleted()
{
	return !bIsRunning && bIsCompleted && !bIsFailed;
}