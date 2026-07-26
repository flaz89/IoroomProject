// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core/Types/IoroomTypes.h"

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "IoroomPlayerState.generated.h"

class USnapSettingsViewModel;
class UMaterialParameterCollection;
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
	
	UFUNCTION()
	void OnRep_StencilSlot();
	
	UFUNCTION()
	void OnRep_AssignedColor();
	
	float GetGridStep() const;
	float GetRotStep() const;
	
	UFUNCTION(BlueprintImplementableEvent, Category="UI")
	void OnViewModelsReady();
	
	UPROPERTY(ReplicatedUsing=OnRep_StencilSlot, BlueprintReadOnly)
	int32 StencilSlot = 0;
	
	UPROPERTY(ReplicatedUsing=OnRep_AssignedColor, BlueprintReadOnly)
	FLinearColor AssignedColor = FLinearColor::White;
	
	// material parameter collection
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialParameterCollection> PlayerColorMPC;
	
	// grid snap start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	EGridSnap GridSnap = EGridSnap::Small;
	
	// rotation snap start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	ERotationSnap RotationSnap = ERotationSnap::Fine;
	
	// ViewModels
	UPROPERTY(Transient)
	TObjectPtr<USnapSettingsViewModel> SnapSettingsViewModel;
	
private:
	
	void InitializePlayerColor();
	void InitializeViewModels();
	void RemoveViewModels();
};
