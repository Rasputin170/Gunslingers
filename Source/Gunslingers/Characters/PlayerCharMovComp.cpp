// Fill out your copyright notice in the Description page of Project Settings.

#include "Gunslingers.h"
#include "PlayerCharacter.h"
#include "PlayerCharMovComp.h"

float UPlayerCharMovComp::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const APlayerCharacter* CharOwner = Cast<APlayerCharacter>(PawnOwner);
	if (CharOwner)
	{
		// Slow down during targeting or crouching
		if (CharOwner->IsAiming() && !IsCrouching())
		{
			MaxSpeed *= CharOwner->GetAimingSpeedModifier();
		}
		else if (CharOwner->IsSprinting())
		{
			MaxSpeed *= CharOwner->GetSprintingSpeedModifier();
		}
	}

	return MaxSpeed;
}
