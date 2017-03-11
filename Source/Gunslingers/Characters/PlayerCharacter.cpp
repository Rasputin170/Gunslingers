#include "Gunslingers.h"
#include "PlayerCharacter.h"
#include "PlayerCharMovComp.h"
#include "GameFramework/InputSettings.h"
#include "World/UsableActor.h"
#include "Items/Weapons/Weapon.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

/************************************************************************/
/* A Gunslinger Character                                               */
/*																		*/
/* Section 0: Constructor, Begin Play, Tick and Input					*/
/* Section 1: Movement													*/
/* Section 2: Weapons													*/
/* Section 3: Interactions												*/
/* Section 4: Status													*/
/************************************************************************/

/************************************************************************/
/* Section 0: Constructor, Begin Play, Tick and Input                   */
/************************************************************************/

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerCharMovComp>(ACharacter::CharacterMovementComponentName))
{

	/* Set size for collision capsule */

	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	//Ignore this channel or it will absorb the trace impacts instead of the skeletal mesh
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	/* Set this character to call Tick() every frame. */

	PrimaryActorTick.bCanEverTick = true;

	/* Set adjustments to movements */

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	// Set turn rates for gamepad input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Adjust jump to make it less floaty
	MoveComp->GravityScale = 1.5f;
	MoveComp->JumpZVelocity = 620;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->MaxWalkSpeedCrouched = 200;

	// Enable crouching
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;

	// Speed modifiers
	AimingSpeedModifier = 0.5f;
	SprintingSpeedModifier = 2.5f;

	/* Set the minimum distance to use an object */

	MaxUseDistance = 500;
	bHasNewFocus = true;

	/* Names as specified in the character skeleton */

	EquippedAttachPoint = TEXT("GripPoint");
	PistolAttachPoint = TEXT("PistolSocket");
	RifleAttachPoint = TEXT("RifleSocket");
	ItemAttachPoint1 = TEXT("ItemSocket1");
	ItemAttachPoint2 = TEXT("ItemSocket2");
	ItemAttachPoint3 = TEXT("ItemSocket3");
	ItemAttachPoint4 = TEXT("ItemSocket4");
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponBlueprint == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("Weapon blueprint missing."));
		return;
	}
	Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponBlueprint);

	/** Attach gun mesh component to Skeleton */

	Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), EquippedAttachPoint);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bWantsToRun && !IsSprinting())
	{
		SetSprinting(true);
	}

	if (Controller && Controller->IsLocalController())
	{
		AUsableActor* Usable = GetUsableInView();

		// End Focus
		if (FocusedUsableActor != Usable)
		{
			if (FocusedUsableActor)
			{
				FocusedUsableActor->OnEndFocus();
			}

			bHasNewFocus = true;
		}

		// Assign new Focus
		FocusedUsableActor = Usable;

		// Start Focus.
		if (Usable)
		{
			if (bHasNewFocus)
			{
				Usable->OnBeginFocus();
				bHasNewFocus = false;
			}
		}
	}
}

void APlayerCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	/* Movement and rotation */

	// Move forward
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	// Mouse Rotation
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	// Gamepad Rotation 
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

	/* Jumping, sprinting and crouching */

	// Jumping
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::OnJump);
	// Crouching
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerCharacter::OnCrouch);
	// Sprinting
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::OnStartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::OnStopSprint);

	/* Weapons interactions */

	// Aiming
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerCharacter::OnStartAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerCharacter::OnEndAim);
	// Firing
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::OnStartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerCharacter::OnStopFire);
	// Reloading
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter::OnReload);
	// Weapon selection
	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &APlayerCharacter::OnNextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &APlayerCharacter::OnPrevWeapon);
	PlayerInputComponent->BindAction("EquipPistol", IE_Pressed, this, &APlayerCharacter::OnEquipPistol);
	PlayerInputComponent->BindAction("EquipRifle", IE_Pressed, this, &APlayerCharacter::OnEquipRifle);
	/*
	PlayerInputComponent->BindAction("EquipItem1", IE_Pressed, this, &APlayerCharacter::OnEquipItem1);
	PlayerInputComponent->BindAction("EquipItem2", IE_Pressed, this, &APlayerCharacter::OnEquipItem2);
	PlayerInputComponent->BindAction("EquipItem3", IE_Pressed, this, &APlayerCharacter::OnEquipItem3);
	PlayerInputComponent->BindAction("EquipItem4", IE_Pressed, this, &APlayerCharacter::OnEquipItem4);
	*/

	/* Interactions */

	// Using
	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &APlayerCharacter::Use);

}

/************************************************************************/
/* Section 1: Movement                                                  */
/************************************************************************/

/* Moving and turning */

void APlayerCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.0f) {
		const bool bLimitRotation = (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling());
		const FRotator Rotation = bLimitRotation ? GetActorRotation() : Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Value != 0.0f) {
		const FRotator Rotation = GetActorRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

/* Jumping */

void APlayerCharacter::OnJump()
{
	SetIsJumping(true);
}

void APlayerCharacter::SetIsJumping(bool NewJumping)
{
	// Go to standing pose if trying to jump while crouched
	if (bIsCrouched && NewJumping) {
		UnCrouch();
	}
	else if (NewJumping != bIsJumping) {
		bIsJumping = NewJumping;
		if (bIsJumping) { Jump(); }
	}

	if (Role < ROLE_Authority)
	{
		ServerSetIsJumping(NewJumping);
	}
}

void APlayerCharacter::ServerSetIsJumping_Implementation(bool NewJumping)
{
	SetIsJumping(NewJumping);
}

bool APlayerCharacter::ServerSetIsJumping_Validate(bool NewJumping) 
{
	return true;
}

bool APlayerCharacter::IsJumping() const
{
	return bIsJumping;
}

/* Crouching */

void APlayerCharacter::OnCrouch()
{
	if (IsSprinting()) { SetSprinting(false); }

	if (CanCrouch()) { Crouch(); }
	else { UnCrouch(); }
}

bool APlayerCharacter::IsCrouching() const
{
	return bIsCrouched;
}

/* Sprinting */

void APlayerCharacter::OnStartSprint()
{
	SetSprinting(true);
}

void APlayerCharacter::OnStopSprint()
{
	SetSprinting(false);
}

void APlayerCharacter::SetSprinting(bool NewSprinting)
{
	//StopWeaponFire();
	bWantsToRun = NewSprinting;

	if (bIsCrouched)
	{
		UnCrouch();
	}

	if (Role < ROLE_Authority)
	{
		ServerSetSprinting(NewSprinting);
	}
}

void APlayerCharacter::ServerSetSprinting_Implementation(bool NewSprinting)
{
	SetSprinting(NewSprinting);
}


bool APlayerCharacter::ServerSetSprinting_Validate(bool NewSprinting)
{
	return true;
}

bool APlayerCharacter::IsSprinting() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	/* Don't allow sprint while aiming, strafing or standing still
	(1.0 is straight forward, -1.0 is backward while near 0 is sideways or standing still)

	Changing this value to 0.1 allows for diagonal sprinting. (holding W+A or W+D keys) */

	return bWantsToRun && !IsAiming() && !GetVelocity().IsZero() && (FVector::DotProduct(GetVelocity().GetSafeNormal2D(), GetActorRotation().Vector()) > 0.8);
}

float APlayerCharacter::GetSprintingSpeedModifier() const
{
	return SprintingSpeedModifier;
}

/* Movement Change Mode */

void APlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	/* Check if we are no longer falling/jumping */
	if (PrevMovementMode == EMovementMode::MOVE_Falling && GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling) {
		SetIsJumping(false);
	}
}

/************************************************************************/
/* Section 2: Weapons                                                   */
/************************************************************************/

/* Firing */

void APlayerCharacter::OnStartFire()
{
	if (IsSprinting()) { SetSprinting(false); }

	if (!bWantsToFire) {
		bWantsToFire = true;
			if (CurrentWeapon) { Weapon->StartFire(); }
	}
}

void APlayerCharacter::OnStopFire()
{
	if (bWantsToFire) {
		bWantsToFire = false;
			if (CurrentWeapon) { Weapon->StopFire(); }
	}
}

bool APlayerCharacter::CanFire() const
{
	/* Add your own checks here, for example non-shooting areas or checking if player is in an NPC dialogue etc. */
	return true;
}

bool APlayerCharacter::IsFiring() const
{
	return Weapon && Weapon->GetCurrentState() == EWeaponState::Firing;
}

void APlayerCharacter::OnReload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
	}
}

/* Aiming */

void APlayerCharacter::OnStartAim()
{
	SetAiming(true);
}

void APlayerCharacter::OnEndAim()
{
	SetAiming(false);
}

float APlayerCharacter::GetAimingSpeedModifier() const
{
	return AimingSpeedModifier;
}

void APlayerCharacter::SetAiming(bool NewAiming)
{
	bIsAiming = NewAiming;

	if (Role < ROLE_Authority)
	{
		ServerSetAiming(NewAiming);
	}
}

void APlayerCharacter::ServerSetAiming_Implementation(bool NewAiming)
{
	SetAiming(NewAiming);
}

bool APlayerCharacter::ServerSetAiming_Validate(bool NewAiming)
{
	return true;
}

FRotator APlayerCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

bool APlayerCharacter::IsAiming() const
{
	return bIsAiming;
}

bool APlayerCharacter::CanReload() const
{
	return true;
}

/************************************************************************/
/* Section 3: Interactions		                                        */
/************************************************************************/

AUsableActor* APlayerCharacter::GetUsableInView()
{
	FVector CamLoc;
	FRotator CamRot;

	if (Controller == nullptr) { return nullptr; }

	Controller->GetPlayerViewPoint(CamLoc, CamRot);
	const FVector TraceStart = CamLoc;
	const FVector Direction = CamRot.Vector();
	const FVector TraceEnd = TraceStart + (Direction * MaxUseDistance);

	FCollisionQueryParams TraceParams(TEXT("TraceUsableActor"), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;

	/* Not tracing complex uses the rough collision instead making tiny objects easier to select. */
	TraceParams.bTraceComplex = false;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f);

	return Cast<AUsableActor>(Hit.GetActor());
}

void APlayerCharacter::Use()
{
	// Only allow on server. If called on client push this request to the server
	if (Role == ROLE_Authority)
	{
		AUsableActor* Usable = GetUsableInView();
		if (Usable)
		{
			Usable->OnUsed(this);
		}
	}
	else
	{
		ServerUse();
	}
}

void APlayerCharacter::ServerUse_Implementation()
{
	Use();
}

bool APlayerCharacter::ServerUse_Validate()
{
	return true;
}

/************************************************************************/
/* Section 4: Status	                                                */
/************************************************************************/

float APlayerCharacter::TakeDamage(float Damage, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	if (Health <= 0.f) { return 0.f; }

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f) {
		Health -= ActualDamage;

		if (Health <= 0) {
			bool bCanDie = true;

			/* Check the damagetype, always allow dying if the cast fails, otherwise check the property if player can die from damagetype */
			if (DamageEvent.DamageTypeClass) {
				UDamageType* DmgType = Cast<UDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
				bCanDie = true; // (DmgType == nullptr || (DmgType && DmgType->GetCanDieFrom()));
			}

			if (bCanDie) { Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser); }
			else { Health = 1.0f; } // Player cannot die from this damage type, set hitpoints to 1.0
		}
		else {
			/* Shorthand for - if x != null pick1 else pick2 */
			APawn* Pawn = EventInstigator ? EventInstigator->GetPawn() : nullptr;
			PlayHit(ActualDamage, DamageEvent, Pawn, DamageCauser, false);
		}
	}

	return ActualDamage;
}

void APlayerCharacter::PlayHit(float DamageTaken, FDamageEvent const & DamageEvent, APawn * PawnInstigator, AActor * DamageCauser, bool bKilled)
{
	if (Role == ROLE_Authority) {
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, bKilled);
	}

	if (GetNetMode() != NM_DedicatedServer) {

		if (bKilled && SoundDeath) { UGameplayStatics::SpawnSoundAttached(SoundDeath, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true); }

		else if (SoundTakeHit) { UGameplayStatics::SpawnSoundAttached(SoundTakeHit, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true); }
	}
}

bool APlayerCharacter::CanDie(float KillingDamage, FDamageEvent const & DamageEvent, AController * Killer, AActor * DamageCauser) const
{
	/* Check if character is already dying, destroyed or if we have authority */
	if (bIsDying ||
		IsPendingKill() ||
		Role != ROLE_Authority ||
		GetWorld()->GetAuthGameMode() == NULL)
	{
		return false;
	}

	return true;
}

bool APlayerCharacter::Die(float KillingDamage, FDamageEvent const & DamageEvent, AController * Killer, AActor * DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	Health = FMath::Min(0.0f, Health);

	/* Fallback to default DamageType if none is specified */
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	/* Notify the gamemode we got killed for scoring and game over state */
	//	AController* KilledPlayer = Controller ? Controller : Cast<AController>(GetOwner());
	//	GetWorld()->GetAuthGameMode<AGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}

void APlayerCharacter::OnDeath(float KillingDamage, FDamageEvent const & DamageEvent, APawn * PawnInstigator, AActor * DamageCauser)
{
	if (bIsDying) { return; }

	DestroyInventory();
	StopAllAnimMontages();

	bReplicateMovement = false;
	bTearOff = true;
	bIsDying = true;

	PlayHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

	DetachFromControllerPendingDestroy();

	/* Disable all collision on capsule */
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	USkeletalMeshComponent* Mesh3P = GetMesh();
	if (Mesh3P)
	{
		Mesh3P->SetCollisionProfileName(TEXT("Ragdoll"));
	}
	SetActorEnableCollision(true);

	SetRagdollPhysics();

	/* Apply physics impulse on the bone of the enemy skeleton mesh we hit (ray-trace damage only) */
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		FPointDamageEvent PointDmg = *((FPointDamageEvent*)(&DamageEvent));
		{
			// TODO: Use DamageTypeClass->DamageImpulse
			Mesh3P->AddImpulseAtLocation(PointDmg.ShotDirection * 12000, PointDmg.HitInfo.ImpactPoint, PointDmg.HitInfo.BoneName);
		}
	}
	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		FRadialDamageEvent RadialDmg = *((FRadialDamageEvent const*)(&DamageEvent));
		{
			Mesh3P->AddRadialImpulse(RadialDmg.Origin, RadialDmg.Params.GetMaxRadius(), 100000 /*RadialDmg.DamageTypeClass->DamageImpulse*/, ERadialImpulseFalloff::RIF_Linear);
		}
	}
}

void APlayerCharacter::FellOutOfWorld(const UDamageType & DmgType)
{
	Die(Health, FDamageEvent(DmgType.GetClass()), NULL, NULL);
}

void APlayerCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;
	USkeletalMeshComponent* Mesh3P = GetMesh();

	if (IsPendingKill()) { bInRagdoll = false; }
	else if (!Mesh3P || !Mesh3P->GetPhysicsAsset()) { bInRagdoll = false; }
	else {
		Mesh3P->SetAllBodiesSimulatePhysics(true);
		Mesh3P->SetSimulatePhysics(true);
		Mesh3P->WakeAllRigidBodies();
		Mesh3P->bBlendPhysics = true;

		bInRagdoll = true;
	}

	UCharacterMovementComponent* CharacterComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
	if (CharacterComp) {
		CharacterComp->StopMovementImmediately();
		CharacterComp->DisableMovement();
		CharacterComp->SetComponentTickEnabled(false);
	}

	if (!bInRagdoll) {
		// Immediately hide the pawn
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else { SetLifeSpan(10.0f); }
}

void APlayerCharacter::ReplicateHit(float DamageTaken, FDamageEvent const & DamageEvent, APawn * PawnInstigator, AActor * DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	//	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if (PawnInstigator == LastTakeHitInfo.PawnInstigator.Get() && LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass) {
		// Same frame damage
		if (bKilled && LastTakeHitInfo.bKilled) { return; } // Redundant death take hit, ignore it

		DamageTaken += LastTakeHitInfo.ActualDamage;
	}

	LastTakeHitInfo.ActualDamage = DamageTaken;
	LastTakeHitInfo.PawnInstigator = Cast<APlayerCharacter>(PawnInstigator);
	LastTakeHitInfo.DamageCauser = DamageCauser;
	LastTakeHitInfo.SetDamageEvent(DamageEvent);
	LastTakeHitInfo.bKilled = bKilled;
	LastTakeHitInfo.EnsureReplication();
}

void APlayerCharacter::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.bKilled) {
		OnDeath(
			LastTakeHitInfo.ActualDamage,
			LastTakeHitInfo.GetDamageEvent(),
			LastTakeHitInfo.PawnInstigator.Get(),
			LastTakeHitInfo.DamageCauser.Get()
		);
	}
	else {
		PlayHit(
			LastTakeHitInfo.ActualDamage,
			LastTakeHitInfo.GetDamageEvent(),
			LastTakeHitInfo.PawnInstigator.Get(),
			LastTakeHitInfo.DamageCauser.Get(),
			LastTakeHitInfo.bKilled
		);
	}
}

void APlayerCharacter::MakePawnNoise(float Loudness)
{
	if (Role == ROLE_Authority)
	{
		/* Make noise to be picked up by PawnSensingComponent by the enemy pawns */
		MakeNoise(Loudness, this, GetActorLocation());
	}

	LastNoiseLoudness = Loudness;
	LastMakeNoiseTime = GetWorld()->GetTimeSeconds();
}


float APlayerCharacter::GetLastNoiseLoudness()
{
	return LastNoiseLoudness;
}


float APlayerCharacter::GetLastMakeNoiseTime()
{
	return LastMakeNoiseTime;
}

/************************************************************************/
/* Section 5: Inventory	                                                */
/************************************************************************/

FName APlayerCharacter::GetInventoryAttachPoint(EInventorySlot Slot) const
{
	/* Return the socket name for the specified storage slot */
	switch (Slot)
	{
	case EInventorySlot::Equipped:
		return EquippedAttachPoint;
	case EInventorySlot::Pistol:
		return PistolAttachPoint;
	case EInventorySlot::Rifle:
		return RifleAttachPoint;
	case EInventorySlot::Item1:
		return ItemAttachPoint1;
	case EInventorySlot::Item2:
		return ItemAttachPoint2;
	case EInventorySlot::Item3:
		return ItemAttachPoint3;
	case EInventorySlot::Item4:
		return ItemAttachPoint4;
	default:
		// Not implemented.
		return "";
	}
}

bool APlayerCharacter::WeaponSlotAvailable(EInventorySlot CheckSlot)
{
	/* Iterate all weapons to see if requested slot is occupied */
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		AWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			if (Weapon->GetStorageSlot() == CheckSlot)
				return false;
		}
	}

	return true;

	/* Special find function as alternative to looping the array and performing if statements
	the [=] prefix means "capture by value", other options include [] "capture nothing" and [&] "capture by reference" */
	//return nullptr == Inventory.FindByPredicate([=](ASWeapon* W){ return W->GetStorageSlot() == CheckSlot; });
}

AWeapon * APlayerCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

void APlayerCharacter::DestroyInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		AWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon, true);
		}
	}
}

void APlayerCharacter::OnNextWeapon()
{
	if (Inventory.Num() >= 2) // TODO: Check for weaponstate.
	{
		const int32 CurrentWeaponIndex = Inventory.IndexOfByKey(CurrentWeapon);
		AWeapon* NextWeapon = Inventory[(CurrentWeaponIndex + 1) % Inventory.Num()];
		EquipWeapon(NextWeapon);
	}
}

void APlayerCharacter::OnPrevWeapon()
{
	if (Inventory.Num() >= 2) // TODO: Check for weaponstate.
	{
		const int32 CurrentWeaponIndex = Inventory.IndexOfByKey(CurrentWeapon);
		AWeapon* PrevWeapon = Inventory[(CurrentWeaponIndex - 1 + Inventory.Num()) % Inventory.Num()];
		EquipWeapon(PrevWeapon);
	}
}

void APlayerCharacter::OnEquipPistol()
{
	if (Inventory.Num() >= 1)
	{
		/* Find first weapon that uses primary slot. */
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			AWeapon* Weapon = Inventory[i];
			if (Weapon->GetStorageSlot() == EInventorySlot::Pistol)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}

void APlayerCharacter::OnEquipRifle()
{
	if (Inventory.Num() >= 2)
	{
		/* Find first weapon that uses secondary slot. */
		for (int32 i = 0; i < Inventory.Num(); i++)
		{
			AWeapon* Weapon = Inventory[i];
			if (Weapon->GetStorageSlot() == EInventorySlot::Rifle)
			{
				EquipWeapon(Weapon);
			}
		}
	}
}

void APlayerCharacter::SetCurrentWeapon(AWeapon * NewWeapon, AWeapon * LastWeapon)
{
	/* Maintain a reference for visual weapon swapping */
	PreviousWeapon = LastWeapon;

	AWeapon* LocalLastWeapon = nullptr;
	if (LastWeapon)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// UnEquip the current
	bool bHasPreviousWeapon = false;
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
		bHasPreviousWeapon = true;
	}

	CurrentWeapon = NewWeapon;

	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);
		/* Only play equip animation when we already hold an item in hands */
		NewWeapon->OnEquipWeapon(bHasPreviousWeapon);
	}
}

void APlayerCharacter::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void APlayerCharacter::AddWeapon(AWeapon * Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);

		// Equip first weapon in inventory
		if (Inventory.Num() > 0 && CurrentWeapon == nullptr)
		{
			EquipWeapon(Inventory[0]);
		}
	}
}

void APlayerCharacter::RemoveWeapon(AWeapon * Weapon, bool bDestroy)
{
	if (Weapon && Role == ROLE_Authority)
	{
		bool bIsCurrent = CurrentWeapon == Weapon;

		if (Inventory.Contains(Weapon))
		{
			Weapon->OnLeaveInventory();
		}
		Inventory.RemoveSingle(Weapon);

		/* Replace weapon if we removed our current weapon */
		if (bIsCurrent && Inventory.Num() > 0)
		{
			SetCurrentWeapon(Inventory[0]);
		}

		/* Clear reference to weapon if we have no items left in inventory */
		if (Inventory.Num() == 0)
		{
			SetCurrentWeapon(nullptr);
		}

		if (bDestroy)
		{
			Weapon->Destroy();
		}
	}
}

void APlayerCharacter::SwapToNewWeaponMesh()
{
	if (PreviousWeapon)
	{
		PreviousWeapon->AttachMeshToPawn(PreviousWeapon->GetStorageSlot());
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->AttachMeshToPawn(EInventorySlot::Equipped);
	}
}

void APlayerCharacter::EquipWeapon(AWeapon * Weapon)
{
	if (Weapon)
	{
		/* Ignore if trying to equip already equipped weapon */
		if (Weapon == CurrentWeapon)
			return;

		if (Role == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			ServerEquipWeapon(Weapon);
		}
	}
}

void APlayerCharacter::ServerEquipWeapon_Implementation(AWeapon * Weapon)
{
	EquipWeapon(Weapon);
}

bool APlayerCharacter::ServerEquipWeapon_Validate(AWeapon * Weapon)
{
	return true;
}

/************************************************************************/
/* NETWORKING SECTION	                                                */
/************************************************************************/

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Value is already updated locally, skip in replication step
	DOREPLIFETIME_CONDITION(APlayerCharacter, bIsJumping, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(APlayerCharacter, bWantsToRun, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(APlayerCharacter, bIsAiming, COND_SkipOwner);

	DOREPLIFETIME(APlayerCharacter, Weapon);
	DOREPLIFETIME(APlayerCharacter, Health);
	DOREPLIFETIME(APlayerCharacter, LastTakeHitInfo);
	DOREPLIFETIME(APlayerCharacter, Inventory);
}