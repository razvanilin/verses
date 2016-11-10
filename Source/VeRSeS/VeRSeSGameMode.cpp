// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "VeRSeS.h"
#include "VeRSeSGameMode.h"
#include "VeRSeSHUD.h"
#include "VeRSeSCharacter.h"

AVeRSeSGameMode::AVeRSeSGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AVeRSeSHUD::StaticClass();
}
