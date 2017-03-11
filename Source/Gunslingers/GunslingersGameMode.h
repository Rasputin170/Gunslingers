// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "GunslingersGameMode.generated.h"

UCLASS(minimalapi)
class AGunslingersGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGunslingersGameMode();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 NumberOfPlayers = 8;

	UPROPERTY(EditDefaultsOnly, Category = "Level Setup")
	TSubclassOf<class ATile> TileBlueprint;
	
	UPROPERTY(EditDefaultsOnly, Category = "Level Setup")
	TSubclassOf<class ATile> WallBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = "Level Setup")
	float TileOffset = 4000.;

protected:

	void SpawnLevelTiles();
	void SpawnLevelWalls();

private:
	int32 NumberOfTiles = 12;
	int32 RotationOffset = 0;

	TArray<FVector> AllocatedTransforms;

	bool IsXDirection;
	bool IsPositive;
	bool IsAllocated;
	
	FRotator TileRotation;
	FVector TileTransform;
	FVector OffsetLocation(bool DirectionX, bool Positive);
	
	void SetRandomTransform();
	void CheckAllocation();

	UWorld* const World = GetWorld();
};