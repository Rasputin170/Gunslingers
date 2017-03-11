// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Gunslingers.h"
#include "GunslingersGameMode.h"
#include "GunslingersHUD.h"
#include "World/Tile.h"
#include "Characters/PlayerCharacter.h"

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

	if (World != NULL)
	{
		SpawnLevelTiles();
		SpawnLevelWalls();
	}
}

void AGunslingersGameMode::SpawnLevelTiles()
{
	NumberOfTiles = 3 * NumberOfPlayers;

	for (int32 i = 0; i < NumberOfTiles; i++) {
			World->SpawnActor<ATile>(TileBlueprint, TileTransform, TileRotation);
			AllocatedTransforms.Add(TileTransform);
			SetRandomTransform();
	}
}

void AGunslingersGameMode::SpawnLevelWalls()
{
	for (FVector AllocatedTile : AllocatedTransforms) {

		TileTransform = AllocatedTile;
		OffsetLocation(true, true);
		CheckAllocation();
		if (IsAllocated == false) { World->SpawnActor<ATile>(WallBlueprint, TileTransform, TileRotation); }
		else { UE_LOG(LogTemp, Warning, TEXT("%f, %f, %f is allocated."), TileTransform.X, TileTransform.Y, TileTransform.Z); }

		TileTransform = AllocatedTile;
		OffsetLocation(true, false);
		CheckAllocation();
		if (IsAllocated == false) { World->SpawnActor<ATile>(WallBlueprint, TileTransform, TileRotation); }
		else { UE_LOG(LogTemp, Warning, TEXT("%f, %f, %f is allocated."), TileTransform.X, TileTransform.Y, TileTransform.Z); }

		TileTransform = AllocatedTile;
		OffsetLocation(false, true);
		CheckAllocation();
		if (IsAllocated == false) { World->SpawnActor<ATile>(WallBlueprint, TileTransform, TileRotation); }
		else { UE_LOG(LogTemp, Warning, TEXT("%f, %f, %f is allocated."), TileTransform.X, TileTransform.Y, TileTransform.Z); }

		TileTransform = AllocatedTile;
		OffsetLocation(false, false);
		CheckAllocation();
		if (IsAllocated == false) { World->SpawnActor<ATile>(WallBlueprint, TileTransform, TileRotation); }
		else { UE_LOG(LogTemp, Warning, TEXT("%f, %f, %f is allocated."), TileTransform.X, TileTransform.Y, TileTransform.Z); }
	}
	return;
}

void AGunslingersGameMode::SetRandomTransform()
{
	IsXDirection = FMath::RandBool();
	IsPositive = FMath::RandBool();
	RotationOffset = FMath::RandRange(0, 3);

	TileRotation.Yaw = 90 * RotationOffset;

	OffsetLocation(IsXDirection, IsPositive);
	
	do {
		CheckAllocation();
		if (IsAllocated == true) { OffsetLocation(IsXDirection, IsPositive); }
	} while (IsAllocated == true);

	return;
}

FVector AGunslingersGameMode::OffsetLocation(bool DirectionX, bool Positive)
{
	if (DirectionX == true && Positive == true) {
		TileTransform.X += TileOffset;
	}
	else if (DirectionX == true && Positive == false) {
		TileTransform.X -= TileOffset;
	}
	else if (DirectionX == false && Positive == true) {
		TileTransform.Y += TileOffset;
	}
	else if (DirectionX == false && Positive == false) {
		TileTransform.Y -= TileOffset;
	}

	return TileTransform;
}

void AGunslingersGameMode::CheckAllocation()
{
	IsAllocated = false;
	for (FVector Allocated : AllocatedTransforms) {
		if (TileTransform == Allocated) {
			IsAllocated = true;

			return;
		}
	}

	return;
}
