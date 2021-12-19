// Copyright Epic Games, Inc. All Rights Reserved.

#include "PracticeBuenoGameMode.h"
#include "PracticeBuenoCharacter.h"
#include "UObject/ConstructorHelpers.h"

APracticeBuenoGameMode::APracticeBuenoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
