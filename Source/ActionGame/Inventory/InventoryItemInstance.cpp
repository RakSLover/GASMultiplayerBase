
#include "Inventory/InventoryItemInstance.h"
#include "AG_Statics.h"
#include "Actors/ItemActor.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

void UInventoryItemInstance::Init(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	ItemStaticDataClass = InItemStaticDataClass;
}

const UItemStaticData* UInventoryItemInstance::GetItemStaticData() const
{
	return UAG_Statics::GetItemStaticData(ItemStaticDataClass);
}

void UInventoryItemInstance::OnRep_Equipped()
{

}

void UInventoryItemInstance::OnEquipped(AActor* InOwner)
{
	if (UWorld* World = InOwner->GetWorld())
	{

		const UItemStaticData* StaticData = GetItemStaticData();

		//сначала создание
		FTransform Transform;

		ItemActor = World->SpawnActorDeferred<AItemActor>(StaticData->ItemActorClass, Transform, InOwner);
		ItemActor->Init(this);
		ItemActor->OnEquipped();
		ItemActor->FinishSpawning(Transform);


		//прикрепление только после успешного создания
		ACharacter* Character = Cast<ACharacter>(InOwner);
		if (USkeletalMeshComponent* SK = Character ? Character->GetMesh() : nullptr)
		{
			ItemActor->AttachToComponent(SK, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StaticData->AttachmentSocket);
		}

		


		TryGrantAbilities(InOwner);
		TryApplyEffects(InOwner);


		bEquipped = true;
	}
}

void UInventoryItemInstance::OnUnequipped(AActor* InOwner)
{
	if (ItemActor) 
	{
		ItemActor->Destroy();
		ItemActor = nullptr;
		
	}

	TryRemoveAbilities(InOwner);
	TryRemoveEffects(InOwner);

	bEquipped = false;

}

void UInventoryItemInstance::OnDropped(AActor* InOwner)
{
	if (ItemActor)
	{
		ItemActor->OnDropped();
	}
	TryRemoveAbilities(InOwner);
	TryRemoveEffects(InOwner);
	bEquipped = false;
}

AItemActor* UInventoryItemInstance::GetItemActor()
{
	return ItemActor;
}

void UInventoryItemInstance::TryGrantAbilities(AActor* InOwner)
{
	if (InOwner && InOwner->HasAuthority())
	{
		if (UAbilitySystemComponent* AC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
		{
			const UItemStaticData* StaticData = GetItemStaticData();

			for (auto ItemAbility : StaticData->GrantedAbilities)
			{
				GrantedAbilityHandles.Add(AC->GiveAbility(FGameplayAbilitySpec(ItemAbility)));
			}
		}
	}
}

void UInventoryItemInstance::TryRemoveAbilities(AActor* InOwner)
{
	if (InOwner && InOwner->HasAuthority())
	{
		if (UAbilitySystemComponent* AC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
		{
			const UItemStaticData* StaticData = GetItemStaticData();

			for (auto AbilityHandle : GrantedAbilityHandles)
			{
				//GrantedAbilityHandles.Add(FGameplayAbilitySpec(ItemAbility));
				AC->ClearAbility(AbilityHandle);


			}
			GrantedAbilityHandles.Empty();
		}
	}
}

void UInventoryItemInstance::TryApplyEffects(AActor* InOwner)
{

	if (UAbilitySystemComponent* AC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
	{

		const UItemStaticData* ItemStaticData = GetItemStaticData();

		FGameplayEffectContextHandle EffectContext = AC->MakeEffectContext();

		for (auto GameplayEffect : ItemStaticData->OnGoingEffects)
		{
			if (!GameplayEffect.Get()) continue;

			FGameplayEffectSpecHandle SpecHandle = AC->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);

			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = AC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

				if (!ActiveGEHandle.WasSuccessfullyApplied())
				{
					UE_LOG(LogTemp, Warning, TEXT("Item %s failed to apply runtime effect %s"), *GetName(), *GetNameSafe(GameplayEffect));
				}
				else
				{
					OngoingEffectHandles.Add(ActiveGEHandle);
				}

			}
		}

	}

}

void UInventoryItemInstance::TryRemoveEffects(AActor* InOwner)
{
	if (UAbilitySystemComponent* AC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
	{

		for (FActiveGameplayEffectHandle ActiveEffectHandle : OngoingEffectHandles)
		{

			if (ActiveEffectHandle.IsValid())
			{
				AC->RemoveActiveGameplayEffect(ActiveEffectHandle);
			}
		}

		OngoingEffectHandles.Empty();
	}
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UInventoryItemInstance, ItemStaticDataClass);
	DOREPLIFETIME(UInventoryItemInstance, bEquipped);
	DOREPLIFETIME(UInventoryItemInstance, ItemActor);
}
