// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Core/Types/IoroomTypes.h"
#include "SnapSettingsViewModel.generated.h"

class AIoroomPlayerState;
/**
 * 
 */
UCLASS(BlueprintType)
class UE_IOROOMPROJECT_API USnapSettingsViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	// setter / getter
	void SetGridSnap(EGridSnap NewGridSnap);
	void SetRotationSnap(ERotationSnap NewRotationSnap);
	EGridSnap GetGridSnap() const;
	ERotationSnap GetRotationSnap() const;
	
	UFUNCTION(BlueprintCallable)
	void CycleGridSnap();
	UFUNCTION(BlueprintCallable)
	void CycleRotationSnap();
	
	// VM created in Playerstate so owner is PlayerState
	AIoroomPlayerState* GetOwningPlayerState() const;

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	EGridSnap GridSnap;
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	ERotationSnap RotationSnap;
};
