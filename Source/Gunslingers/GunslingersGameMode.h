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

	void SpawnLevelTiles();

	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 NumberOfPlayers = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Level Setup")
	TSubclassOf<class ATile> TileBlueprint;
	
	UPROPERTY(EditDefaultsOnly, Category = "Level Setup")
	float TileOffset = 2000.;

private:
	int32 NumberOfTiles = 12;

	TArray<FVector> AllocatedTransforms;

	bool IsXDirection;
	bool IsPositive;
	bool IsAllocated;
	
	FRotator TileRotation;
	FVector TileTransform;
	FVector OffsetLocation(bool IsXDirection, bool IsPositive);
	
	void SetNewTransform();
	void CheckAllocation();

};