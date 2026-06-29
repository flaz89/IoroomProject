// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/IoroomPlayerState.h"

#include "Core/IoroomGameState.h"
#include "Engine/World.h"
#include "Kismet/KismetMaterialLibrary.h"
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
	
	UWorld* World = GetWorld();
	if (!World) return;

	AIoroomGameState* IoroomGameState = World->GetGameState<AIoroomGameState>();
	if (!IoroomGameState) return;
	
	StencilSlot = IoroomGameState->ReserveStencilSlot();

	
	AssignedColor = 
		UKismetMaterialLibrary::GetVectorParameterValue(
			GetWorld(), PlayerColorMPC, FName(FString::Printf(TEXT("PlayerColor_%d"), StencilSlot - 1))
			);
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
 * Both properties use ReplicatedUsing to cover all replication orderings.
 * OnRep_StencilSlot handles the case where StencilSlot arrives after AssignedColor.
 * OnRep_AssignedColor handles the case where AssignedColor arrives after StencilSlot.
 * ADesktopPawn::OnRep_PlayerState handles the case where the pawn link arrives last.
 */
void AIoroomPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIoroomPlayerState, AssignedColor)
	DOREPLIFETIME(AIoroomPlayerState, StencilSlot)
}

void AIoroomPlayerState::OnRep_StencilSlot()
{
	ADesktopPawn* Pawn = GetPawn<ADesktopPawn>();
	if (Pawn) Pawn->ApplyPlayerVisuals(AssignedColor);
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
