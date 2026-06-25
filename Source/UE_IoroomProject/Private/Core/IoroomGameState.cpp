// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/IoroomGameState.h"

AIoroomGameState::AIoroomGameState()
{
	// initialized empty array lenght 10
	UsedStencilSlots.Init(false, MaxPlayers); 
}

int32 AIoroomGameState::ReserveStencilSlot()
{
	const int32 FreeIndex = UsedStencilSlots.IndexOfByKey(false);
	if (FreeIndex == INDEX_NONE) return -1;
	
	UsedStencilSlots[FreeIndex] = true;
	return FreeIndex + MinStencilSlot;
}

void AIoroomGameState::ReleaseStencilSlot(int32 StencilSlot)
{
	const int32 Index = StencilSlot - MinStencilSlot;
	if (UsedStencilSlots.IsValidIndex(Index)) UsedStencilSlots[Index] = false;
}
