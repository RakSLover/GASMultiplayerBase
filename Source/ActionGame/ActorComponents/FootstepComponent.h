
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AG_Types.h"
#include "FootstepComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONGAME_API UFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UFootstepComponent();

protected:
	
	virtual void BeginPlay() override;




public:	
	
	void HandleFootstep(EFoot Foot);

	UPROPERTY(EditDefaultsOnly)
	FName LeftFootSocketName = TEXT("foot_l");

	UPROPERTY(EditDefaultsOnly)
	FName RightFootSocketName = TEXT("foot_r");
	
};
