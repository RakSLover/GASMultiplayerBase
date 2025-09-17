

#include "AbilitySystem/Abilities/GA_InventoryCombatAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AttributeSets/AG_AttributeSetBase.h"
#include "AG_Types.h"

#include "Inventory/ItemActors/WeaponItemActor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ActionGameCharacter.h"
#include"Camera/CameraComponent.h"

FGameplayEffectSpecHandle UGA_InventoryCombatAbility::GetWeaponEffectSpec(const FHitResult& InHitResult)
{

	if (UAbilitySystemComponent* AC = GetAbilitySystemComponentFromActorInfo())
	{
		if (const UWeaponStaticData* WSD = GetEquippedItemWeaponStaticData())
		{
			FGameplayEffectContextHandle EC = AC->MakeEffectContext();
			
			FGameplayEffectSpecHandle OutSpec = AC->MakeOutgoingSpec(WSD->DamageEffect, 1, EC);

			//UE_LOG(LogTemp, Warning, TEXT("Damage magnitude: %f"), -WSD->BaseDamage);


			//динамическое изменение величины эффекта
			//с помощью тега устанвока значения для желаемого эффекта
			//запрос тега по имени
		
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(OutSpec, FGameplayTag::RequestGameplayTag(TEXT("Attribute.Health")), -WSD->BaseDamage);
			return  OutSpec;

		}

	}


	return FGameplayEffectSpecHandle();
}

const bool UGA_InventoryCombatAbility::GetWeaponToFocusTraceResult(float TraceDistance, ETraceTypeQuery TraceType, FHitResult& OutHitResult)
{

	//первая трассировка для камеры
	// вторая трассировка от оружия до конечной точки трассировки камеры

	AWeaponItemActor* WeaponItemActtor = GetEquippedWeaponItemActor();
	AActionGameCharacter* AGChar = GetActionGameCharacterFromActorInfo();
	const FTransform& CameraTransform = AGChar->GetFollowCamera()->GetComponentTransform();
	const FVector FocusTraceEnd = CameraTransform.GetLocation() + CameraTransform.GetRotation().Vector() * TraceDistance;
	TArray<AActor*> ActorsToIgnore;// = { GetAvatarActorFromActorInfo() };
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());

	FHitResult FocusHit;
	UKismetSystemLibrary::LineTraceSingle(this, CameraTransform.GetLocation(), FocusTraceEnd, TraceType, false, ActorsToIgnore, EDrawDebugTrace::None, FocusHit, true);

	FVector MuzzleLocation = WeaponItemActtor->GetMuzzleLocation();
	const FVector WeaponTraceEnd = MuzzleLocation + (FocusHit.Location - MuzzleLocation).GetSafeNormal() * TraceDistance;
	UKismetSystemLibrary::LineTraceSingle(this, MuzzleLocation, WeaponTraceEnd, TraceType, false, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHitResult, true);

	return OutHitResult.bBlockingHit;

}
