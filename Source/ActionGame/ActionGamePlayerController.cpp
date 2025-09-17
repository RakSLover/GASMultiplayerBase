// Copyright Epic Games, Inc. All Rights Reserved.


#include "ActionGamePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "ActionGame.h"

#include "AbilitySystemComponent.h"
#include "ActionGameGameMode.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "Widgets/Input/SVirtualJoystick.h"

void AActionGamePlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AActionGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AActionGamePlayerController::RestartPlayerIn(float InTime)
{
	//удалит существующего игрока из контроллера игрока, персонаж останется в мире
	ChangeState(NAME_Spectating);

	GetWorld()->GetTimerManager().SetTimer(RestartPlayerTimerHandle, this, &AActionGamePlayerController::RestartPlayer, InTime, false);
}

void AActionGamePlayerController::OnPossess(APawn* APawn)
{
	Super::OnPossess(APawn);

	//подпись на изменение состояния смерти
	if (UAbilitySystemComponent* AC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(APawn))
	{
		DeathStateTagDelegate = AC->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(TEXT("State.Dead")),
			EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AActionGamePlayerController::OnPawnDeathStateChanged);
	}

}

void AActionGamePlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	// отписать от события смерти
	if (DeathStateTagDelegate.IsValid())
	{
		if (UAbilitySystemComponent* AC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
		{
			AC->UnregisterGameplayTagEvent(DeathStateTagDelegate, FGameplayTag::RequestGameplayTag(TEXT("State.Dead")), EGameplayTagEventType::NewOrRemoved);
		}
	}

}

void AActionGamePlayerController::OnPawnDeathStateChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0) 
	{
		UWorld* World = GetWorld();
		AActionGameGameMode* GameMode = World ? Cast<AActionGameGameMode>(World->GetAuthGameMode()) : nullptr;
		if (GameMode)
		{
			//уведомить гейм мод для респавна
			GameMode->NotifyPlayerDied(this);
		}


		// отписать от события смерти
		if (DeathStateTagDelegate.IsValid())
		{
			if (UAbilitySystemComponent* AC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
			{
				AC->UnregisterGameplayTagEvent(DeathStateTagDelegate, FGameplayTag::RequestGameplayTag(TEXT("State.Dead")), EGameplayTagEventType::NewOrRemoved);
			}
		}


	}
}

void AActionGamePlayerController::RestartPlayer()
{
	UWorld* World = GetWorld();
	AActionGameGameMode* GameMode = World ? Cast<AActionGameGameMode>(World->GetAuthGameMode()) : nullptr;
	if (GameMode)
	{
		GameMode->RestartPlayer(this);
	}

}
