
#include "Inventory/InventoryList.h"
#include "Inventory/InventoryItemInstance.h"
#include "AG_Types.h"

void FInventoryList::AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	//создаем новый элемент
	FInventoryListItem& Item = Items.AddDefaulted_GetRef();
	Item.ItemInstance = NewObject<UInventoryItemInstance>();

	Item.ItemInstance->Init(InItemStaticDataClass);
	//метка как измененный
	MarkItemDirty(Item);
    


}

void FInventoryList::AddItem(UInventoryItemInstance* InItemInstance)
{
    
	FInventoryListItem& Item = Items.AddDefaulted_GetRef();
	Item.ItemInstance = InItemInstance;
	MarkItemDirty(Item);   

    // Проверяем нет ли уже такого предмета
    /*
    for (auto& Item : Items)
    {
        if (Item.ItemInstance == InItemInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("Item already in inventory!"));
            return; // Уже есть - не добавляем
        }
    }

    // Добавляем только если еще нет
    FInventoryListItem& Item = Items.AddDefaulted_GetRef();
    Item.ItemInstance = InItemInstance;
    MarkItemDirty(Item);
    */

}

void FInventoryList::RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	
	for (auto ItemIter = Items.CreateIterator(); ItemIter; ++ItemIter) 
	{
		FInventoryListItem& Item = *ItemIter;
		if (Item.ItemInstance && Item.ItemInstance->GetItemStaticData()->IsA(InItemStaticDataClass))
		//if (Item.ItemInstance && Item.ItemInstance->ItemStaticDataClass == InItemStaticDataClass)
		{
			//удаление текущего элемента из итератора
			ItemIter.RemoveCurrent();
			MarkArrayDirty();
			break;

		}
	}

}
