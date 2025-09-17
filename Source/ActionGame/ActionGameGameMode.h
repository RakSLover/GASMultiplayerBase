
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ActionGameGameMode.generated.h"

class AActionGamePlayerController;

UCLASS(abstract)
class AActionGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	AActionGameGameMode();

	void NotifyPlayerDied(AActionGamePlayerController* PC);


};



