#pragma once
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

/************************************************************************/
/* A Gunslinger Character                                               */
/*																		*/
/* Section 0: Constructor, Begin Play, Tick and Input					*/
/* Section 1: Movement													*/
/* Section 2: Weapons													*/
/* Section 3: Interactions												*/
/* Section 4: Status													*/
/* Section 5: Inventory													*/
/************************************************************************/

/************************************************************************/
/* Section 0: Constructor, Begin Play, Tick and Input                   */
/************************************************************************/

/**
* Inventory and equipment slots for weapons
*/

UENUM()
enum class EInventorySlot : uint8
{
	Equipped,
	Pistol,
	Rifle,
	Item1,
	Item2,
	Item3,
	Item4,
};

/**
* Struct to store the hit information
*/

USTRUCT()
struct FTakeHitInfo
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		float ActualDamage;

	UPROPERTY()
		UClass* DamageTypeClass;

	UPROPERTY()
		TWeakObjectPtr<class APlayerCharacter> PawnInstigator;

	UPROPERTY()
		TWeakObjectPtr<class AActor> DamageCauser;

	UPROPERTY()
		uint8 DamageEventClassID;

	UPROPERTY()
		bool bKilled;

private:

	UPROPERTY()
		uint8 EnsureReplicationByte;

	UPROPERTY()
		FDamageEvent GeneralDamageEvent;

	UPROPERTY()
		FPointDamageEvent PointDamageEvent;

	UPROPERTY()
		FRadialDamageEvent RadialDamageEvent;

public:
	FTakeHitInfo()
		: ActualDamage(0),
		DamageTypeClass(nullptr),
		PawnInstigator(nullptr),
		DamageCauser(nullptr),
		DamageEventClassID(0),
		bKilled(false),
		EnsureReplicationByte(0)
	{}

	FDamageEvent& GetDamageEvent()
	{
		switch (DamageEventClassID)
		{
		case FPointDamageEvent::ClassID:
			if (PointDamageEvent.DamageTypeClass == nullptr)
			{
				PointDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
			}
			return PointDamageEvent;

		case FRadialDamageEvent::ClassID:
			if (RadialDamageEvent.DamageTypeClass == nullptr)
			{
				RadialDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
			}
			return RadialDamageEvent;

		default:
			if (GeneralDamageEvent.DamageTypeClass == nullptr)
			{
				GeneralDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
			}
			return GeneralDamageEvent;
		}
	}


	void SetDamageEvent(const FDamageEvent& DamageEvent)
	{
		DamageEventClassID = DamageEvent.GetTypeID();
		switch (DamageEventClassID)
		{
		case FPointDamageEvent::ClassID:
			PointDamageEvent = *((FPointDamageEvent const*)(&DamageEvent));
			break;
		case FRadialDamageEvent::ClassID:
			RadialDamageEvent = *((FRadialDamageEvent const*)(&DamageEvent));
			break;
		default:
			GeneralDamageEvent = DamageEvent;
		}
	}


	void EnsureReplication()
	{
		EnsureReplicationByte++;
	}
};

/**
* Player constructor begins here
*/

class UInputComponent;

UCLASS(config = Game)
class GUNSLINGERS_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void StopAllAnimMontages();

	//UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SoundTakeHit;

	//UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SoundDeath;

private:

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/************************************************************************/
	/* Section 1: Movement                                                  */
	/************************************************************************/

public:

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsJumping() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual bool IsSprinting() const;

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool IsCrouching() const;

	float GetSprintingSpeedModifier() const;

	// Movement
	void MoveForward(float Val);
	void MoveRight(float Val);

	// Turning with gamepad
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	// Crouching
	void OnCrouch();

	// Sprinting
	void OnStartSprint();
	void OnStopSprint();

	void SetSprinting(bool NewSprinting);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(bool NewSprinting);

	void ServerSetSprinting_Implementation(bool NewSprinting);

	bool ServerSetSprinting_Validate(bool NewSprinting);

	/* Character wants to run, checked during Tick to see if allowed */
	UPROPERTY(Transient, Replicated)
	bool bWantsToRun;

	// Jumping
	void OnJump();

	void SetIsJumping(bool NewJumping);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSetIsJumping(bool NewJumping);

	void ServerSetIsJumping_Implementation(bool NewJumping);

	bool ServerSetIsJumping_Validate(bool NewJumping);

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	/* Is character currently performing a jump action. Resets on landed.  */
	UPROPERTY(Transient, Replicated)
	bool bIsJumping;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		float SprintingSpeedModifier;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		float AimingSpeedModifier;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		float BaseTurnRate;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		float BaseLookUpRate;

	/************************************************************************/
	/* Section 2: Weapons	                                                 */
	/************************************************************************/

public:

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<class AWeapon> WeaponBlueprint;

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool IsFiring() const;

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool IsAiming() const;

	bool CanFire() const;

	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	FRotator GetAimOffsets() const;

	void SetAiming(bool NewAiming);

	float GetAimingSpeedModifier() const;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetAiming(bool NewAiming);

	void ServerSetAiming_Implementation(bool NewAiming);

	bool ServerSetAiming_Validate(bool NewAiming);

	UPROPERTY(Transient, Replicated)
		bool bIsFiring;

	UPROPERTY(Transient, Replicated)
		bool bIsAiming;

private:

	AWeapon* Weapon;

	bool bWantsToFire;

	void OnStartFire();
	void OnStopFire();

	void OnReload();

	void OnStartAim();
	void OnEndAim();

	/************************************************************************/
	/* Section 3: Interactions		                                         */
	/************************************************************************/

private:

	/* Use the usable actor currently in focus */
	virtual void Use();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUse();

	void ServerUse_Implementation();

	bool ServerUse_Validate();

	/* Max distance to use actors */
	UPROPERTY(EditDefaultsOnly, Category = "Interactions")
		float MaxUseDistance;

	/* True only in first frame when focused on a new usable actor */
	bool bHasNewFocus;

	class AUsableActor* FocusedUsableActor;

	class AUsableActor* GetUsableInView();

	/************************************************************************/
	/* Section 4: Status	                                                 */
	/************************************************************************/

public:

	UFUNCTION(BlueprintCallable, Category = "Status")
	void MakePawnNoise(float Loudness);
	float GetLastNoiseLoudness();
	float GetLastMakeNoiseTime();

	float LastNoiseLoudness;
	float LastMakeNoiseTime;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Status", Replicated)
		float Health;

	/* Take damage & handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled);

	bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	bool Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser);

	void OnDeath(float KillingDamage, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser);

	virtual void FellOutOfWorld(const class UDamageType& DmgType) override;

	void SetRagdollPhysics();

	void ReplicateHit(float DamageTaken, struct FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser, bool bKilled);

	/* Holds hit data to replicate hits and death to clients */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	FDamageEvent LastDamageEvent;

	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	bool bIsDying;

	/************************************************************************/
	/* Section 5: Inventory		                                         */
	/************************************************************************/

public:

	/* All weapons/items the player currently holds */
	UPROPERTY(Transient, Replicated)
	TArray<AWeapon*> Inventory;

	/* Return socket name for attachments (to match the socket in the character skeleton) */
	FName GetInventoryAttachPoint(EInventorySlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeapon* GetCurrentWeapon() const;

	/* Check if the specified slot is available, limited to one item per type (primary, secondary) */
	bool WeaponSlotAvailable(EInventorySlot CheckSlot);

	void DestroyInventory();

	void SetCurrentWeapon(class AWeapon* NewWeapon, class AWeapon* LastWeapon = nullptr);

	void EquipWeapon(AWeapon* Weapon);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEquipWeapon(AWeapon* Weapon);

	void ServerEquipWeapon_Implementation(AWeapon* Weapon);

	bool ServerEquipWeapon_Validate(AWeapon* Weapon);

	/* OnRep functions can use a parameter to hold the previous value of the variable. Very useful when you need to handle UnEquip etc. */
	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* LastWeapon);

	void AddWeapon(class AWeapon* Weapon);
	void RemoveWeapon(class AWeapon* Weapon, bool bDestroy);

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
		class AWeapon* CurrentWeapon;

	class AWeapon* PreviousWeapon;

	/* Update the weapon mesh to the newly equipped weapon, this is triggered during an anim montage.
	NOTE: Requires an AnimNotify created in the Equip animation to tell us when to swap the meshes. */
	UFUNCTION(BlueprintCallable, Category = "Animation")
		void SwapToNewWeaponMesh();

private:
	/* Attach points for active weapons */

	// Weapon in hands
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
		FName EquippedAttachPoint;
	// Pistol carried on the belt
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
		FName PistolAttachPoint;
	// Rifle carried on the back
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
		FName RifleAttachPoint;
	// Items carried on the belt
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
		FName ItemAttachPoint1;
	// Items carried on the belt
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
		FName ItemAttachPoint2;
	// Items carried on the belt
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
		FName ItemAttachPoint3;
	// Items carried on the belt
	UPROPERTY(EditDefaultsOnly, Category = "Sockets")
		FName ItemAttachPoint4;

	/* Equip items and weapons */

	void OnNextWeapon();
	void OnPrevWeapon();
	void OnEquipPistol();
	void OnEquipRifle();
	//	void OnEquipItem1();
	//	void OnEquipItem2();
	//	void OnEquipItem3();
	//	void OnEquipItem4();

};

