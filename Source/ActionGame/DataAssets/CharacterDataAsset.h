
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AG_Types.h"
#include "CharacterDataAsset.generated.h"


UCLASS(BlueprintType, Blueprintable)
class ACTIONGAME_API UCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FCharacterData CharacterData;
	
};
