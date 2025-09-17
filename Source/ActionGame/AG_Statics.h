
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include"AG_Types.h"
#include "AG_Statics.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGAME_API UAG_Statics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static const UItemStaticData* GetItemStaticData(TSubclassOf <UItemStaticData> ItemDataClass);
	

	
	
};
