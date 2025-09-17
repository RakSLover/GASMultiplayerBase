
#pragma once

#include "CoreMinimal.h"
#include "AG_Statics.h"
//#include "NetCoreClasses.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Net/UnrealNetwork.h"
#include "InventoryList.generated.h"

class UInventoryItemInstance;

// ������������

USTRUCT(BlueprintType)
struct FInventoryListItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:

	// ���������� ����������� ���� �� ����������
	UPROPERTY()
	UInventoryItemInstance* ItemInstance = nullptr;

};

USTRUCT(BlueprintType)
struct FInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()



	TArray<FInventoryListItem>& GetItemsRef() { return Items; }

	//����� ����������� ��� ������ ������������
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams) 
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryListItem, FInventoryList>(Items, DeltaParams, *this);
	}

	// �������� ������� �� ����������� ������
	void AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass); 

	// �������� ������� �� ����������
	void AddItem(UInventoryItemInstance* InItemInstance);

	void RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);

protected:

	UPROPERTY()
	TArray<FInventoryListItem> Items;


	

};


//...
template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum {WithNetDeltaSerializer = true};
};
