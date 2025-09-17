// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemInterface.h"
#include "AG_Types.h"
#include "ActorComponents/InventoryComponent.h"
#include "DataAssets/CharacterDataAsset.h"
#include "ActionGameCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;


class UAG_AbilitySystemComponentBase;
class UAG_AttributeSetBase;

class UGameplayEffect;
class UGameplayAbility;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);


UCLASS(abstract)
class AActionGameCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:


	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;


	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;


	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;


	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DropItemAction;


	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* EquipNextItemAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UnequipItemAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AimAction;

public:

	//AActionGameCharacter();	

	AActionGameCharacter(const FObjectInitializer& ObjectInitializer);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PostLoad() override;

	bool ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext);



protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GiveAbilities();
	void ApplyStartupEffects();

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	virtual void Landed(const FHitResult& Hit) override;

	virtual void OnStartCrouch(float HalfHeightAdhust, float ScaledHalfHeightAdhust) override;

	virtual void OnEndCrouch(float HalfHeightAdhust, float ScaledHalfHeightAdhust) override;


	UPROPERTY(EditDefaultsOnly)
	UAG_AbilitySystemComponentBase* AbilitySystemComponent;

	UPROPERTY(Transient)
	UAG_AttributeSetBase* AttributeSet;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnCrouchActionStarted();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnCrouchActionEnded();


	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnSprintActionStarted();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnSprintActionEnded();


	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnDropItemTriggered();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnEquipNextItemTriggered();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnUnequipItemTriggered();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnAttackStarted();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnAttackEnded();


	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnAimStarted();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void OnAimEnded();


public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


public:

	UFUNCTION(BlueprintCallable)
	FCharacterData GetCharacterData() const;

	UFUNCTION(BlueprintCallable)
	void SetCharacterData(const FCharacterData& InCharacterData);

	class UFootstepComponent* GetFootstepComponent() const;

	void MaxMovementSpeedChanged(const FOnAttributeChangeData& Data);

	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);

	void StartRagdoll();
	

protected:

	UPROPERTY(ReplicatedUsing = OnRep_CharacterData)
	FCharacterData CharacterData;

	UFUNCTION()
	void OnRep_CharacterData();

	virtual void InitFromCharacterData(const FCharacterData& InCharacterData, bool bFromReplication = false);

	UPROPERTY(EditDefaultsOnly)
	class UCharacterDataAsset* CharacterDataAsset;

	UPROPERTY(BlueprintReadOnly)
	class UFootstepComponent* FootstepComponent;

	UFUNCTION()
	void OnRagdollStateTagChanged(const FGameplayTag CallabckTag, int32 NewCount);

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag JumpEventTag;


	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AttackStartedEventTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AttackEndedEventTag;


	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AimStartedEventTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AimEndedEventTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ZeroHealthEventTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag RagdollStateTag;


	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> CrouchStateEffect;


	//gameplay tags

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer InAirTags;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer CrouchTags;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer SprintTags;


	//delegates

	FDelegateHandle MaxMovementSpeedChangedDelegateHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	UInventoryComponent* InventoryComponent = nullptr;

};

