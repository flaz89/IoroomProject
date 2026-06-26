// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/IoroomPlayerState.h"

#include "Core/IoroomGameState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

void AIoroomPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority()) return;

	AIoroomGameState* IoroomGameState = GetWorld()->GetGameState<AIoroomGameState>();
	if (IoroomGameState)
	{
		StencilSlot = IoroomGameState->ReserveStencilSlot();
		
		const uint8 Hue = static_cast<uint8>((StencilSlot -1) * 25);
		AssignedColor = FLinearColor::MakeFromHSV8(Hue, 200, 200);
	}
}

void AIoroomPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (HasAuthority())
	{
		AIoroomGameState* IoroomGameState = GetWorld()->GetGameState<AIoroomGameState>();
		if (IoroomGameState) IoroomGameState->ReleaseStencilSlot(StencilSlot);
	}
}

void AIoroomPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AIoroomPlayerState, AssignedColor)
	DOREPLIFETIME(AIoroomPlayerState, StencilSlot)
}

void AIoroomPlayerState::OnRep_AssignedColor()
{
	//TODO need to replicate AssignedColor to Dynamic material on Pawn body mesh
}
