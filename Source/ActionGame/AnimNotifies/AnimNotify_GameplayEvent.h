
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AnimNotify_GameplayEvent.generated.h"

UCLASS()
class ACTIONGAME_API UAnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
	
	
public:

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	UPROPERTY(EditAnywhere)
	FGameplayEventData Payload;
	
};
