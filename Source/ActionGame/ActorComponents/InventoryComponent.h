
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryList.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "InventoryComponent.generated.h"


//ItemStaticData - ������ ������(����� ��� ����)
//ItemInstance - ���������� ������(� ��������, ����������)
//InventoryComponent - �����(������ ���� ������)
//ItemInstance (������) - ItemActor (���������� �������������)



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UInventoryComponent();

	virtual void InitializeComponent() override;

	//��� ��������� �������� ���������
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	//�������� �� �������
	UFUNCTION(BlueprintCallable)
	void AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);

	//�������� �� ����������
	UFUNCTION(BlueprintCallable)
	void AddItemInstance(UInventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable)
	void RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);

	//����������� ������� �� �������, ��������� �������� �������, ~IDItem or other
	UFUNCTION(BlueprintCallable)
	void EquipItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);

	//�� ����������
	UFUNCTION(BlueprintCallable)
	void EquipItemInstance(UInventoryItemInstance* InItemInstance);

	//����� �������, ��������� �������� �������, ~IDItem or other
	UFUNCTION(BlueprintCallable)
	void UnequipItem();

	UFUNCTION(BlueprintCallable)
	void DropItem();

	//
	UFUNCTION(BlueprintCallable)
	void EquipNext();


	UFUNCTION(BlueprintCallable, BlueprintPure)
	UInventoryItemInstance* GetEquippedItem() const;

	virtual void GameplayEventCallback(const FGameplayEventData* Payload);

	// ���� ��� ��������������
	static FGameplayTag EquipItemActorTag;
	static FGameplayTag DropItemTag;
	static FGameplayTag EquipNextTag;
	static FGameplayTag UnequipItemTag;

protected:

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(Replicated)
	FInventoryList InventoryList;

protected:

	UFUNCTION()
	void AddInventoryTags();


	//�������� ��������� �� ������
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UItemStaticData>> DefaultItems;

	UPROPERTY(Replicated)
	UInventoryItemInstance* CurrentItem = nullptr;

	//��� ��������������� ������� ����������� � ���� ���������
	void HandleGameplayEventInternal(FGameplayEventData Payload);


	UFUNCTION(Server, Reliable)
	void ServerHandleGameplayEvent(FGameplayEventData Payload);
	virtual void ServerHandleGameplayEvent_Implementation(FGameplayEventData Payload);
	
};
