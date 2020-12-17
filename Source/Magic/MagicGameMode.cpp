// Copyright Epic Games, Inc. All Rights Reserved.

#include "MagicGameMode.h"
#include "MagicCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMagicGameMode::AMagicGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
