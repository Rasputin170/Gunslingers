// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Gunslingers.h"
#include "GunslingersGameMode.h"
#include "GunslingersHUD.h"
#include "World/Tile.h"
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

void AGunslingersGameMode::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	SpawnLevelTiles();
}

void AGunslingersGameMode::SpawnLevelTiles()
{
	UWorld* const World = GetWorld();
	if (World != NULL)
	{
		for (int32 i = 0; i < NumberOfTiles; i++) {
			World->SpawnActor<ATile>(TileBlueprint, TileTransform, TileRotation);
			AllocatedTransforms.Add(TileTransform);
			SetNewTransform();
		}
	}
}

void AGunslingersGameMode::SetNewTransform()
{
	IsXDirection = FMath::RandBool();
	IsPositive = FMath::RandBool();

	OffsetLocation(IsXDirection, IsPositive);
	
	do {
		IsAllocated = false; 
		CheckAllocation();
		if (IsAllocated == true) { OffsetLocation(IsXDirection, IsPositive); }
	} while (IsAllocated == true);

	return;
}

FVector AGunslingersGameMode::OffsetLocation(bool IsXDirection, bool IsPositive)
{
	if (IsXDirection == true && IsPositive == true) {
		TileTransform.X += TileOffset;
	}
	else if (IsXDirection == true && IsPositive == false) {
		TileTransform.X -= TileOffset;
	}
	else if (IsXDirection == false && IsPositive == true) {
		TileTransform.Y += TileOffset;
	}
	else if (IsXDirection == false && IsPositive == false) {
		TileTransform.Y -= TileOffset;
	}

	return TileTransform;
}

void AGunslingersGameMode::CheckAllocation()
{
	for (FVector Allocated : AllocatedTransforms) {
		if (TileTransform == Allocated) {
			IsAllocated = true;

			return;
		}
	}
	return;
}
