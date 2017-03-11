// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharMovComp.generated.h"

/**
 * 
 */
UCLASS()
class GUNSLINGERS_API UPlayerCharMovComp : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	virtual float GetMaxSpeed() const override;
	
};
