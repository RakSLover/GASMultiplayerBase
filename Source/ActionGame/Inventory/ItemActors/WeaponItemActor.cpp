
#include "Inventory/ItemActors/WeaponItemActor.h"
#include "Inventory/InventoryItemInstance.h"
#include "AG_Types.h"

AWeaponItemActor::AWeaponItemActor()
{

}

const UWeaponStaticData* AWeaponItemActor::GetWeaponStaticData() const
{
	return ItemInstance ? Cast<UWeaponStaticData>(ItemInstance->GetItemStaticData()) : nullptr;
}

FVector AWeaponItemActor::GetMuzzleLocation() const
{
	return MeshComponent ? MeshComponent->GetSocketLocation(TEXT("Muzzle")) : GetActorLocation();
}

void AWeaponItemActor::InitInternal()
{
	Super::InitInternal();
	if (const UWeaponStaticData* WeaponData = GetWeaponStaticData())
	{
		if (WeaponData->SkeletalMesh)
		{
			//меш будет создан на сервере и на клиенте, без репликации
			USkeletalMeshComponent* SC = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), TEXT("MeshComponent"));
			if (SC)
			{
				SC->RegisterComponent();
				SC->SetSkeletalMesh(WeaponData->SkeletalMesh);
				SC->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				SC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				MeshComponent = SC;
			}
		}

		else if(WeaponData->StaticMesh)
		{
			UStaticMeshComponent* SC = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("MeshComponent"));
			if (SC)
			{
				SC->RegisterComponent();
				SC->SetStaticMesh(WeaponData->StaticMesh);
				SC->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				SC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				MeshComponent = SC;
			}
		}

	}
}
