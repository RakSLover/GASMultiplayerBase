
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AG_Types.h"
#include "GameplayTagContainer.h"
#include "AG_CharacterMovementComponent.generated.h"



UCLASS()
class ACTIONGAME_API UAG_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintPure)
	EMovementDirectionType GetMovementDirectionType() const;

	UFUNCTION(BlueprintCallable)
	void SetMovementDirectionType(EMovementDirectionType InMovementDirectionType);

	//функция обратного вызова
	UFUNCTION()
	void OnEnforcedStrafeTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

protected:

	UPROPERTY(EditAnywhere)
	EMovementDirectionType MovementDirectionType;

	//обработка типа движения
	void HandleMovementDirection();
	
};
