// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Gunslingers.h"
#include "GunslingersGameMode.h"
#include "GunslingersHUD.h"
#include "GunslingersCharacter.h"

AGunslingersGameMode::AGunslingersGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Dynamic/Characters/Player/Player_BP"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AGunslingersHUD::StaticClass();
}
