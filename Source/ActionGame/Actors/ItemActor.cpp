
#include "Actors/ItemActor.h"

#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Inventory/InventoryItemInstance.h"
#include "ActorComponents/InventoryComponent.h"

AItemActor::AItemActor()
{

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->InitSphereRadius(32.0f);
	//SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ƒл€ отладки сделаем видимым
	//SphereComponent->SetHiddenInGame(false);
	//SphereComponent->SetVisibility(true);


	//SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnSphereOverlap);
	//SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnSphereOverlap);

}

void AItemActor::OnEquipped()
{
	ItemState = EItemState::Equipped;
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//SphereComponent->SetGenerateOverlapEvents(false);
	
}

void AItemActor::OnUnequipped()
{
	ItemState = EItemState::None;
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItemActor::OnDropped()
{
	ItemState = EItemState::Dropped;
	
	//SphereComponent->SetGenerateOverlapEvents(true);

	GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	SetActorLocation(this->GetActorLocation() + FVector(250.f, 250.f, 0.f));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);


	//трассировка чтобы положить на пол 
}

bool AItemActor::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	//реплицировать подобъект предмет
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);

	
	return WroteSomething;
}

void AItemActor::Init(UInventoryItemInstance* InInstance)
{
	ItemInstance = InInstance;

	InitInternal();
}


void AItemActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (SphereComponent)
	{

		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnSphereOverlap);
		

	}

		if (HasAuthority())
		{
			if (!IsValid(ItemInstance) && IsValid(ItemStaticDataClass))
			{
				ItemInstance = NewObject<UInventoryItemInstance>();
				ItemInstance->Init(ItemStaticDataClass);

			}
		}

		//SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		//SphereComponent->SetGenerateOverlapEvents(true);

		InitInternal();

}

void AItemActor::OnRep_ItemInstance(UInventoryItemInstance* OldItemInstance)
{
	if (IsValid(ItemInstance) && !IsValid(OldItemInstance)) //впервые получили экземпл€р
	{
		InitInternal();
	}
}


void AItemActor::InitInternal()
{

}

void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemActor::OnRep_ItemState()
{
	switch (ItemState)
	{

	case EItemState::Equipped:
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetGenerateOverlapEvents(false);
		
		break;


	default:
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComponent->SetGenerateOverlapEvents(true);
		
		break;
	}
}

void AItemActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (HasAuthority())
	{
		FGameplayEventData EventPayload;
		EventPayload.Instigator = this;
		EventPayload.OptionalObject = ItemInstance;
		EventPayload.EventTag = UInventoryComponent::EquipItemActorTag;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, UInventoryComponent::EquipItemActorTag, EventPayload);
	}


}


void AItemActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemActor, ItemState);
	DOREPLIFETIME(AItemActor, ItemInstance);
}

