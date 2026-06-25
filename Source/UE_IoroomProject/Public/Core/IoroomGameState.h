// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "IoroomGameState.generated.h"

/**
 * 
 */
UCLASS()
class UE_IOROOMPROJECT_API AIoroomGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AIoroomGameState();
	
	int32 ReserveStencilSlot();
	void ReleaseStencilSlot(int32 StencilSlot);
	
private:
	static constexpr int32 MinStencilSlot = 1; // PPV stencil values selected starts from 1 to 10
	static constexpr int32 MaxPlayers = 10;
	
	TArray<bool> UsedStencilSlots;
};
