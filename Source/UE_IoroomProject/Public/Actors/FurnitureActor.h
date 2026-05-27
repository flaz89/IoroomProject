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
	
	void OnHovered();
	void OnUnHovered();
	void OnSelected();
	void OnDeselected();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FurnitureMesh;
	
	UPROPERTY(EditDefaultsOnly, Category="Material")
	TObjectPtr<UMaterialInterface> MouseHoverMaterial;
	
private:
	bool bIsSelected = false;
	
};
