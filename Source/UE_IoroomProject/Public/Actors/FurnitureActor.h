// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FurnitureActor.generated.h"

UCLASS()
class UE_IOROOMPROJECT_API AFurnitureActor : public AActor
{
	GENERATED_BODY()

public:
	AFurnitureActor();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	bool IsSelectableFurniture() const { return SelectingStencilSlot == 0; }
	
	void OnHovered(UMaterialInterface* OverlayMaterial);
	void OnUnHovered();
	void OnSelected(int32 InStencilSlot);
	void OnDeselected();
	
	FBox GetMeshBoundingBox() const;
	
	

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FurnitureMesh;
	
private:
	
	UPROPERTY(ReplicatedUsing=OnRep_SelectingStencilSlot)
	int32 SelectingStencilSlot = 0;
	
	UFUNCTION()
	void OnRep_SelectingStencilSlot();
	
};
