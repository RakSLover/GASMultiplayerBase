
#include "ActorComponents/AG_CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"


void UAG_CharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	HandleMovementDirection();

	if (UAbilitySystemComponent* AC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())) 
	{
		//подписка на изменение тэга
		AC->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(TEXT("Movement.Enforced.Strafe"),
			EGameplayTagEventType::NewOrRemoved))
			.AddUObject(this, &UAG_CharacterMovementComponent::OnEnforcedStrafeTagChanged);
	}

}

EMovementDirectionType UAG_CharacterMovementComponent::GetMovementDirectionType() const
{
	return MovementDirectionType;

}

void UAG_CharacterMovementComponent::SetMovementDirectionType(EMovementDirectionType InMovementDirectionType)
{
	MovementDirectionType = InMovementDirectionType;

	HandleMovementDirection();
}

void UAG_CharacterMovementComponent::OnEnforcedStrafeTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount) 
	{
		SetMovementDirectionType(EMovementDirectionType::Strafe);
	}
	else
	{
		SetMovementDirectionType(EMovementDirectionType::OrientToMovement);
	}
}

void UAG_CharacterMovementComponent::HandleMovementDirection()
{
	switch (MovementDirectionType)
	{
		break;
	case EMovementDirectionType::Strafe:
		bUseControllerDesiredRotation = true;
		bOrientRotationToMovement = false;
		CharacterOwner->bUseControllerRotationYaw = true;
		break;
	default:
		bUseControllerDesiredRotation = false;
		bOrientRotationToMovement = true;
		CharacterOwner->bUseControllerRotationYaw = false;
		break;
	}



}

