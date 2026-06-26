// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/IoroomGameMode.h"

#include "Core/IoroomGameState.h"
#include "Core/IoroomPlayerController.h"
#include "Player/DesktopPawn.h"
#include "Player/IoroomPlayerState.h"

AIoroomGameMode::AIoroomGameMode()
{
	DefaultPawnClass = ADesktopPawn::StaticClass();
	PlayerControllerClass = AIoroomPlayerController::StaticClass();
	GameStateClass = AIoroomGameState::StaticClass();
	PlayerStateClass = AIoroomPlayerState::StaticClass();
}
