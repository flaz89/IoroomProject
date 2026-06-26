// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/IoroomPlayerState.h"

#include "Core/IoroomGameState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Player/DesktopPawn.h"

/*
 * Server-only: reserves a stencil slot from the GameState pool and derives a unique hue from it.
 * Slot-based hue (step of 25/255) guarantees visually distinct colors across all 10 players
 * without risking duplicates that a random approach would produce.
 */
void AIoroomPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	AIoroomGameState* IoroomGameState = GetWorld()->GetGameState<AIoroomGameState>();
	if (IoroomGameState)
	{
		StencilSlot = IoroomGameState->ReserveStencilSlot();

		const uint8 Hue = static_cast<uint8>((StencilSlot - 1) * 25);
		AssignedColor = FLinearColor::MakeFromHSV8(Hue, 200, 200);
	}
}

/*
 * Releases the stencil slot back to the pool when the player leaves,
 * making it available for the next incoming player.
 */
void AIoroomPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (HasAuthority())
	{
		AIoroomGameState* IoroomGameState = GetWorld()->GetGameState<AIoroomGameState>();
		if (IoroomGameState) IoroomGameState->ReleaseStencilSlot(StencilSlot);
	}
}

/*
 * AssignedColor uses ReplicatedUsing to trigger a visual update on all clients when it changes.
 * StencilSlot uses plain Replicated since clients read it on demand (furniture selection/hover),
 * so no immediate callback is needed.
 */
void AIoroomPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIoroomPlayerState, AssignedColor)
	DOREPLIFETIME(AIoroomPlayerState, StencilSlot)
}

/*
 * Fires on clients when AssignedColor replicates. Covers the case where AssignedColor arrives
 * after PawnPrivate is already set. The reverse case (PawnPrivate arrives after AssignedColor)
 * is handled by ADesktopPawn::OnRep_PlayerState.
 */
void AIoroomPlayerState::OnRep_AssignedColor()
{
	ADesktopPawn* Pawn = GetPawn<ADesktopPawn>();
	if (Pawn) Pawn->ApplyPlayerVisuals(AssignedColor);
}
