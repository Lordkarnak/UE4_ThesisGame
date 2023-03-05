// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "ParticleDefinitions.h"
#include "SoundDefinitions.h"
//#include "ThesisGameGameMode.h"
//#include "ThesisGameGameState.h"
//#include "ThesisGameHUD.h"
//#include "ThesisGameCharacter.h"
//#include "ThesisSaveGame.h"
//#include "Characters/PlayableCharacter.h"
//#include "Characters/PlayableController.h"
//#include "Quests/Quest.h"

#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
#define COLLISION_PICKUP		ECC_GameTraceChannel3

namespace EGamePhysMaterialType
{
	enum Type
	{
		Unknown,
		Concrete,
		Dirt,
		Water,
		Metal,
		Wood,
		Grass,
		Glass,
		Flesh
	};
}

#define SHOOTER_SURFACE_Default		SurfaceType_Default
#define SHOOTER_SURFACE_Concrete	SurfaceType1
#define SHOOTER_SURFACE_Dirt		SurfaceType2
#define SHOOTER_SURFACE_Water		SurfaceType3
#define SHOOTER_SURFACE_Metal		SurfaceType4
#define SHOOTER_SURFACE_Wood		SurfaceType5
#define SHOOTER_SURFACE_Grass		SurfaceType6
#define SHOOTER_SURFACE_Glass		SurfaceType7
#define SHOOTER_SURFACE_Flesh		SurfaceType8