

#include "ActorComponents/FootstepComponent.h"
#include "ActionGameCharacter.h"
#include "PhysicalMaterials/AG_BasePhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"


UFootstepComponent::UFootstepComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;

}



void UFootstepComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UFootstepComponent::HandleFootstep(EFoot Foot)
{
	if (AActionGameCharacter* Character = Cast<AActionGameCharacter>(GetOwner())) 
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh()) 
		{
		
			FHitResult HitResult;
			const FVector SocketLocation = Mesh->GetSocketLocation(Foot == EFoot::Left ? LeftFootSocketName : RightFootSocketName);
			const FVector Location = SocketLocation + FVector::UpVector * 20;

			FCollisionQueryParams QueryParams;
			QueryParams.bReturnPhysicalMaterial = true;
			QueryParams.AddIgnoredActor(Character);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, Location, Location + FVector::UpVector * -50.f, ECollisionChannel::ECC_WorldStatic, QueryParams))
			{
				
				if(HitResult.bBlockingHit)
				{
					
					if (HitResult.PhysMaterial.Get())
					{
						
						UAG_BasePhysicalMaterial* PM = Cast<UAG_BasePhysicalMaterial>(HitResult.PhysMaterial.Get());

						if (PM)
						{
							UGameplayStatics::PlaySoundAtLocation(this, PM->FootstepSound, Location, 1.f);
							
						}
						
					}
				}
			}
		}
	}
}

