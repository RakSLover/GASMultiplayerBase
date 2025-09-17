
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AG_Types.h"
#include "GameplayAbilitySpec.h"
#include "InventoryItemInstance.generated.h"

class AItemActor;

UCLASS(BlueprintType, Blueprintable)
class ACTIONGAME_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:

	virtual void Init(TSubclassOf<UItemStaticData> InItemStaticDataClass);
	
	virtual bool IsSupportedForNetworking() const override { return true; }


	UPROPERTY(Replicated)
	TSubclassOf<UItemStaticData> ItemStaticDataClass;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const UItemStaticData* GetItemStaticData() const;

	UPROPERTY(ReplicatedUsing = OnRep_Equipped)
	bool bEquipped = false;

	UFUNCTION()
	void OnRep_Equipped();

	virtual void OnEquipped(AActor* InOwner = nullptr);

	virtual void OnUnequipped(AActor* InOwner = nullptr);

	virtual void OnDropped(AActor* InOwner = nullptr);

	UFUNCTION(BlueprintPure)
	AItemActor* GetItemActor();

protected:

	UPROPERTY(Replicated)
	AItemActor* ItemActor = nullptr;

	//при экипировки предмета даем способности и удаляем в обратном случае
	void TryGrantAbilities(AActor* InOwner);

	void TryRemoveAbilities(AActor* InOwner);

	void TryApplyEffects(AActor* InOwner);

	void TryRemoveEffects(AActor* InOwner);

	//массив дескрипторов способностей
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	//массив эффектов которые дает предмет
	//UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TArray<FActiveGameplayEffectHandle> OngoingEffectHandles;
	
};
