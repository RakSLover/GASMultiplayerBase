
#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "AG_BasePhysicalMaterial.generated.h"


UCLASS()
class ACTIONGAME_API UAG_BasePhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PhysicalMaterial)
	class USoundBase* FootstepSound = nullptr;
	
	
};
