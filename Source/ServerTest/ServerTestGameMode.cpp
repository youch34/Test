// Copyright Epic Games, Inc. All Rights Reserved.

#include "ServerTestGameMode.h"
#include "ServerTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

AServerTestGameMode::AServerTestGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
