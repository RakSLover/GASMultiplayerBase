
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AG_Types.h"
#include "AnimNotify_Footstep.generated.h"



UCLASS()
class ACTIONGAME_API UAnimNotify_Footstep : public UAnimNotify
{
	GENERATED_BODY()
	
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	UPROPERTY(EditAnywhere)
	EFoot Foot;
	
};
