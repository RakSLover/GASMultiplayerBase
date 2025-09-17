
#include "ActionGameGameMode.h"
#include "ActionGamePlayerController.h"

AActionGameGameMode::AActionGameGameMode()
{

}

void AActionGameGameMode::NotifyPlayerDied(AActionGamePlayerController* PC)
{
	if (PC)
	{
		PC->RestartPlayerIn(2.f);
	}
}
