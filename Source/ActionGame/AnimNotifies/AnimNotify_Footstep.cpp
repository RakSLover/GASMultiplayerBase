// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/AnimNotify_Footstep.h"
#include "ActionGameCharacter.h"
#include "ActorComponents/FootstepComponent.h"

void UAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	check(MeshComp);

	AActionGameCharacter* Character = MeshComp ? Cast<AActionGameCharacter>(MeshComp->GetOwner()) : nullptr;

	if (Character)
	{
		if (UFootstepComponent* FT = Character->GetFootstepComponent())
		{
			FT->HandleFootstep(Foot);
		}
	}

}
