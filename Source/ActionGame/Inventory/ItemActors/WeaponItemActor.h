
#pragma once

#include "CoreMinimal.h"
#include "Actors/ItemActor.h"
#include "WeaponItemActor.generated.h"


UCLASS()
class ACTIONGAME_API AWeaponItemActor : public AItemActor
{
	GENERATED_BODY()
	
public:

	AWeaponItemActor();
	

	const UWeaponStaticData* GetWeaponStaticData() const;

	UFUNCTION(BlueprintPure)
	FVector GetMuzzleLocation() const;

protected:

	UPROPERTY()
	UMeshComponent* MeshComponent = nullptr;

	virtual void InitInternal() override;
	
};
