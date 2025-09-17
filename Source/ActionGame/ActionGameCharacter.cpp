// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGameCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AttributeSets/AG_AttributeSetBase.h"
#include "AbilitySystem/Components/AG_AbilitySystemComponentBase.h"
#include "GameplayEffectExtension.h"
#include "ActorComponents/AG_CharacterMovementComponent.h"
#include "ActorComponents/FootstepComponent.h"
#include "Net/UnrealNetwork.h"
#include "ActionGame.h"


AActionGameCharacter::AActionGameCharacter(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UAG_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	//переопределение базового мув компонента

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);


	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;


	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;


	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	


	//ability system

	AbilitySystemComponent = CreateDefaultSubobject<UAG_AbilitySystemComponentBase>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAG_AttributeSetBase>(TEXT("AttributeSet"));

	FootstepComponent = CreateDefaultSubobject<UFootstepComponent>(TEXT("FootstepComponent"));

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);



	// привязка изменения аттрибута, гибкий подход для изменения скорости, все изменения скорорсти будут применятся как игровые эффекты, без лишнего кода ун-го мод-ка скорости
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxMovementSpeedAttribute())
		.AddUObject(this, &AActionGameCharacter::MaxMovementSpeedChanged);

	//привязка изменения атрибута здоровья
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
		.AddUObject(this, &AActionGameCharacter::OnHealthAttributeChanged);

	//слежка за изменением тега состояния
	AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(TEXT("State.Ragdoll")),
		EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AActionGameCharacter::OnRagdollStateTagChanged);

}

UAbilitySystemComponent* AActionGameCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AActionGameCharacter::PostLoad()
{
	Super::PostLoad();

	if (IsValid(CharacterDataAsset)) 
	{
		SetCharacterData(CharacterDataAsset->CharacterData);
	}

}


bool AActionGameCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext)
{

	if (!Effect.Get()) return false;

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, InEffectContext);

	if (SpecHandle.IsValid()) 
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		return ActiveGEHandle.WasSuccessfullyApplied();
	}

	return false;
}

void AActionGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AActionGameCharacter::DoJumpStart);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);


		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::Look);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnCrouchActionStarted);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnCrouchActionEnded);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::OnSprintActionStarted);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnSprintActionEnded);

		EnhancedInputComponent->BindAction(DropItemAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnDropItemTriggered);
		EnhancedInputComponent->BindAction(EquipNextItemAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnEquipNextItemTriggered);
		EnhancedInputComponent->BindAction(UnequipItemAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnUnequipItemTriggered);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnAttackStarted);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnAttackEnded);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnAimStarted);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnAimEnded);

	}
	else
	{
		UE_LOG(LogActionGame, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AActionGameCharacter::GiveAbilities()
{
	// выдача способностей в начале игры

	if (HasAuthority() && AbilitySystemComponent) 
	{
		for (auto DefaultAbility : CharacterData.Abilities)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DefaultAbility));
		}
	}

}

void AActionGameCharacter::ApplyStartupEffects()
{
	// начальные эффекты при старте персонажа
	if (GetLocalRole() == ROLE_Authority)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		for (auto CharacterEffect : CharacterData.Effects)
		{
			ApplyGameplayEffectToSelf(CharacterEffect, EffectContext);
		}

	}


}

void AActionGameCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	GiveAbilities();
	ApplyStartupEffects();
}

void AActionGameCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

}

void AActionGameCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// удаление всех тегов которые связаны с тегом inair

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveEffectsWithTags(InAirTags);
	}
}

void AActionGameCharacter::OnStartCrouch(float HalfHeightAdhust, float ScaledHalfHeightAdhust)
{
	Super::OnStartCrouch(HalfHeightAdhust, ScaledHalfHeightAdhust);



	if (!CrouchStateEffect.Get()) return;



	if (AbilitySystemComponent)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(CrouchStateEffect, 1, EffectContext);


		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			if (!ActiveGEHandle.WasSuccessfullyApplied())
			{

				UE_LOG(LogTemp, Warning, TEXT("Ability %s failed to apply crouch effect %s"), *GetName(), *GetNameSafe(CrouchStateEffect));
			}

		}
	}





}

void AActionGameCharacter::OnEndCrouch(float HalfHeightAdhust, float ScaledHalfHeightAdhust)
{
	Super::OnEndCrouch(HalfHeightAdhust, ScaledHalfHeightAdhust);


	if (AbilitySystemComponent && CrouchStateEffect.Get())
	{
		//AbilitySystemComponent->RemoveActiveEffectsWithTags(InCrouchTags);
		// 
		//удаление толко текущего эффекта не трогая другие
		AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(CrouchStateEffect, AbilitySystemComponent);
	}




}

void AActionGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AActionGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AActionGameCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AActionGameCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AActionGameCharacter::DoJumpStart()
{
	// signal the character to jump
	//Jump();
	FGameplayEventData Payload;
	Payload.Instigator = this;
	Payload.EventTag = JumpEventTag;

	//вызов способности прыжка по тегу
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, JumpEventTag, Payload);

}

void AActionGameCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	//StopJumping();
}

void AActionGameCharacter::OnCrouchActionStarted()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(CrouchTags, true);
	}
	

	/*
	FGameplayEventData Payload;
	Payload.Instigator = this;
	Payload.EventTag = CrouchEventTag;

	//вызов способности прыжка по тегу
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, CrouchEventTag, Payload);
	*/

}

void AActionGameCharacter::OnCrouchActionEnded()
{
	if (AbilitySystemComponent)
	{
		//AbilitySystemComponent->RemoveActiveEffectsWithTags(InAirTags);
		AbilitySystemComponent->CancelAbilities(&CrouchTags);
	}
}

void AActionGameCharacter::OnSprintActionStarted()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(SprintTags, true);
	}
}

void AActionGameCharacter::OnSprintActionEnded()
{

	if (AbilitySystemComponent)
	{
		//AbilitySystemComponent->RemoveActiveEffectsWithTags(SprintTags);
		AbilitySystemComponent->CancelAbilities(&SprintTags);
		
	}
}

void AActionGameCharacter::OnDropItemTriggered()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = UInventoryComponent::DropItemTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UInventoryComponent::DropItemTag, EventPayload);


}

void AActionGameCharacter::OnEquipNextItemTriggered()
{
		
	FGameplayEventData EventPayload;
	EventPayload.EventTag = UInventoryComponent::EquipNextTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UInventoryComponent::EquipNextTag, EventPayload);
	
	
}

void AActionGameCharacter::OnUnequipItemTriggered()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = UInventoryComponent::UnequipItemTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UInventoryComponent::UnequipItemTag, EventPayload);

}

void AActionGameCharacter::OnAttackStarted()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AttackStartedEventTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AttackStartedEventTag, EventPayload);
}

void AActionGameCharacter::OnAttackEnded()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AttackEndedEventTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AttackEndedEventTag, EventPayload);
}

void AActionGameCharacter::OnAimStarted()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AimStartedEventTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AimStartedEventTag, EventPayload);
	
}

void AActionGameCharacter::OnAimEnded()
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AimEndedEventTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AimEndedEventTag, EventPayload);

}


FCharacterData AActionGameCharacter::GetCharacterData() const
{
	return CharacterData;
}

void AActionGameCharacter::SetCharacterData(const FCharacterData& InCharacterData)
{
	CharacterData = InCharacterData;

	InitFromCharacterData(CharacterData, true);
}

UFootstepComponent* AActionGameCharacter::GetFootstepComponent() const
{
	return FootstepComponent;
}

void AActionGameCharacter::MaxMovementSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void AActionGameCharacter::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{

	if (Data.NewValue <= 0 && Data.OldValue > 0)
	{
		AActionGameCharacter* OtherCharacter = nullptr;

		if (Data.GEModData)
		{
			// получить контекст эффекта (для получения данных, кто нанес урон и т.п)
			const FGameplayEffectContextHandle& EC = Data.GEModData->EffectSpec.GetEffectContext();

			// получить зачинщика урона из контекста эффекта
			OtherCharacter = Cast<AActionGameCharacter>(EC.GetInstigator());
			
		}

		FGameplayEventData EventPayload;
		EventPayload.EventTag = ZeroHealthEventTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, ZeroHealthEventTag, EventPayload);	

	}
}




void AActionGameCharacter::OnRep_CharacterData()
{
	InitFromCharacterData(CharacterData, true);
}

void AActionGameCharacter::InitFromCharacterData(const FCharacterData& InCharacterData, bool bFromReplication)
{

}

void AActionGameCharacter::OnRagdollStateTagChanged(const FGameplayTag CallabckTag, int32 NewCount)
{
	if (NewCount > 0) 
	{
		StartRagdoll();
	}
}

void AActionGameCharacter::StartRagdoll()
{
	USkeletalMeshComponent* SK = GetMesh();

	if (SK && !SK->IsSimulatingPhysics())
	{
		SK->SetCollisionProfileName(TEXT("Ragdoll"));
		SK->SetSimulatePhysics(true);
		SK->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
		SK->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		SK->WakeAllRigidBodies();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	}

}

void AActionGameCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AActionGameCharacter, CharacterData);
	DOREPLIFETIME(AActionGameCharacter, InventoryComponent);

}