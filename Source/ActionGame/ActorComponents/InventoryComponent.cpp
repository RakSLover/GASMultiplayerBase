
#include "ActorComponents/InventoryComponent.h"
#include "Inventory/InventoryList.h"
#include "Inventory/InventoryItemInstance.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayTagsManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Actors/ItemActor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

//статические переменные
FGameplayTag UInventoryComponent::EquipItemActorTag;
FGameplayTag UInventoryComponent::DropItemTag;
FGameplayTag UInventoryComponent::EquipNextTag;
FGameplayTag UInventoryComponent::UnequipItemTag;


static TAutoConsoleVariable<int32> CVarShowInventory
(
	TEXT("ShowDebugInventory"),
	0,
	TEXT("Draws debug info about inventory")
	TEXT(" 0: of/n")
	TEXT(" 1: on/n"),
	ECVF_Cheat
);


UInventoryComponent::UInventoryComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

	//репликаци€ дл€ предметов со старта по умолчанию 
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);

	//инициализаци€ тегов с дальнейшей подписью
	UGameplayTagsManager::Get().OnLastChanceToAddNativeTags().AddUObject(this, &UInventoryComponent::AddInventoryTags);

}


void UInventoryComponent::AddInventoryTags()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

	UInventoryComponent::EquipItemActorTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.EquipItemActor"), TEXT("Equip item from item actor event"));
	UInventoryComponent::DropItemTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.DropItem"), TEXT("Drop equipped item"));
	UInventoryComponent::EquipNextTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.EquipNext"), TEXT("Try equip next item"));
	UInventoryComponent::UnequipItemTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.Unequip"), TEXT("Unequip current item"));

	//отписка от событи€
	TagsManager.OnLastChanceToAddNativeTags().RemoveAll(this);
}

void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();


	if (GetOwner()->HasAuthority())
	{

		for (auto ItemClass : DefaultItems)
		{
			InventoryList.AddItem(ItemClass);
		}



		if (InventoryList.GetItemsRef().Num() > 0)
		{
			EquipItem(InventoryList.GetItemsRef()[0].ItemInstance->ItemStaticDataClass);
		}
		//DropItem();

	}



		//подпись на игровые событи€
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::EquipItemActorTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::DropItemTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::EquipNextTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::UnequipItemTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);

	}

}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	//репликаци€ каждого экземпл€ра предмета
	for (FInventoryListItem& Item : InventoryList.GetItemsRef())
	{
		UInventoryItemInstance* ItemInstance = Item.ItemInstance;

		if (IsValid(ItemInstance))
		{
			WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}
	return WroteSomething;
}

void UInventoryComponent::AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.AddItem(InItemStaticDataClass);
	}
}

void UInventoryComponent::AddItemInstance(UInventoryItemInstance* InItemInstance)
{

	if (GetOwner()->HasAuthority())
	{
		InventoryList.AddItem(InItemInstance);
	}

}

void UInventoryComponent::RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.RemoveItem(InItemStaticDataClass);
	}
}

void UInventoryComponent::EquipItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		
		//поиск предмета дл€ экипировки из инвентар€
		for (auto Item : InventoryList.GetItemsRef())
		{
			if (Item.ItemInstance->ItemStaticDataClass == InItemStaticDataClass)
			{
				Item.ItemInstance->OnEquipped(GetOwner());
				CurrentItem = Item.ItemInstance;
				
				break;
			}
		}
	}
}

void UInventoryComponent::EquipItemInstance(UInventoryItemInstance* InItemInstance)
{

	if (GetOwner()->HasAuthority())
	{
		for (auto Item : InventoryList.GetItemsRef())
		{
			if (Item.ItemInstance == InItemInstance) 
			{
				Item.ItemInstance->OnEquipped(GetOwner());
				CurrentItem = Item.ItemInstance;
				
				break;
			}
		}
	}
}

void UInventoryComponent::UnequipItem()
{
	if (GetOwner()->HasAuthority())
	{
		if(IsValid(CurrentItem))
		{

			CurrentItem->OnUnequipped(GetOwner());
			CurrentItem = nullptr;
		}
	}

}

void UInventoryComponent::DropItem()
{
	if (GetOwner()->HasAuthority())
	{
		if (IsValid(CurrentItem))
		{
			CurrentItem->OnDropped(GetOwner());
			RemoveItem(CurrentItem->ItemStaticDataClass);
			CurrentItem = nullptr;
		}
	}
}

void UInventoryComponent::EquipNext()
{
	
	TArray<FInventoryListItem>& Items = InventoryList.GetItemsRef();
	const bool bNoItems = Items.Num() == 0; //не пустой инвентарь
	const bool bOneAndEquipped = Items.Num() == 1 && CurrentItem; // если есть
	if (bNoItems || bOneAndEquipped) return;

	UInventoryItemInstance* TargetItem = CurrentItem;
	//пробежатьс€ по всем предметам
	for (auto Item : Items)
	{
		if (Item.ItemInstance->GetItemStaticData()->bCanEquipped) //может ли предмет быть экипирован (настраиваема€ статика)
		{
			if (Item.ItemInstance != CurrentItem) // нельз€ экипировать этот же предмет
			{
				TargetItem = Item.ItemInstance;
				break;
			}
		}
	}
	if (CurrentItem)
	{
		if (TargetItem == CurrentItem)
		{
			return;
		}
		UnequipItem();
	}
	EquipItemInstance(TargetItem);

}

UInventoryItemInstance* UInventoryComponent::GetEquippedItem() const
{
	return CurrentItem;
}

void UInventoryComponent::GameplayEventCallback(const FGameplayEventData* Payload)
{
	ENetRole NetRole = GetOwnerRole();

	if (GetOwner()->HasAuthority())
	{
		HandleGameplayEventInternal(*Payload);
		

	}
	else if(NetRole == ROLE_AutonomousProxy)
	{
		ServerHandleGameplayEvent(*Payload);
		
	}
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}



void UInventoryComponent::HandleGameplayEventInternal(FGameplayEventData Payload)
{

	ENetRole NetRole = GetOwnerRole();

	if (NetRole == ROLE_Authority)
	{
		FGameplayTag EventTag = Payload.EventTag;

		if (EventTag == UInventoryComponent::EquipItemActorTag)
		{
			
			// получить экземпл€р из предмета (полезна€ нагрузка)
			if (const UInventoryItemInstance* ItemInstance = Cast<UInventoryItemInstance>(Payload.OptionalObject)) 
			{
				
				//удалит константу из объекта
				AddItemInstance(const_cast<UInventoryItemInstance*>(ItemInstance));


				if (Payload.Instigator)
				{
					AActor* InstigatorActor = const_cast<AActor*>(Payload.Instigator.Get());
					//AActor* InstigatorActor = const_cast<AActor*>(Payload.Instigator);
					if (InstigatorActor) 
					{
						InstigatorActor->Destroy();
					}
					//уничтожить предмет
					//const_cast<AActor*>(Payload.Instigator)->Destroy();
				
				}
			}
		}
		else if (EventTag == UInventoryComponent::EquipNextTag)
		{
			EquipNext();
		}
		else if (EventTag == UInventoryComponent::DropItemTag)
		{
			DropItem();
		}
		else if (EventTag == UInventoryComponent::UnequipItemTag)
		{
			UnequipItem();
		}

	}

}

void UInventoryComponent::ServerHandleGameplayEvent_Implementation(FGameplayEventData Payload)
{
	HandleGameplayEventInternal(Payload);
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const bool bShowDebug = CVarShowInventory.GetValueOnGameThread() != 0;
	if (bShowDebug)
	{
		for (FInventoryListItem& Item : InventoryList.GetItemsRef())
		{
			UInventoryItemInstance* ItemInstance = Item.ItemInstance;
			const UItemStaticData* ItemStaticData = ItemInstance->GetItemStaticData();

			if (IsValid(ItemInstance) && IsValid(ItemStaticData))
			{
				GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, FString::Printf(TEXT("Item: %s"), *ItemStaticData->Name.ToString()));
			}
		}
	}

}




void UInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UInventoryComponent, InventoryList); // CurrentItem included
	DOREPLIFETIME(UInventoryComponent, CurrentItem);
}

