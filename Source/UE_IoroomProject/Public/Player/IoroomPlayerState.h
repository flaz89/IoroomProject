// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "IoroomPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class UE_IOROOMPROJECT_API AIoroomPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 StencilSlot = 0;
	
	UPROPERTY(ReplicatedUsing=OnRep_AssignedColor, BlueprintReadOnly)
	FLinearColor AssignedColor = FLinearColor::White;
	
	UFUNCTION()
	void OnRep_AssignedColor();
	
	
	
};
