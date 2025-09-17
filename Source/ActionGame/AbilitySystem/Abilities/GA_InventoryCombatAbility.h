
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_InventoryAbility.h"
#include "GA_InventoryCombatAbility.generated.h"


UCLASS()
class ACTIONGAME_API UGA_InventoryCombatAbility : public UGA_InventoryAbility
{
	GENERATED_BODY()
	
public:

	//для установки урона с помощью тега
	UFUNCTION(BlueprintPure)
	FGameplayEffectSpecHandle GetWeaponEffectSpec(const FHitResult& InHitResult);
	
	UFUNCTION(BlueprintPure)
	const bool GetWeaponToFocusTraceResult(float TraceDistance, ETraceTypeQuery TraceType, FHitResult& OutHitResult);
	
};
