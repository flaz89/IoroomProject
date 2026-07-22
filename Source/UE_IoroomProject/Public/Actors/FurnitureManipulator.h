// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FurnitureManipulator.generated.h"

class AFurnitureActor;

UENUM()
enum class EManipulatorHandle : uint8
{
	None,
	MoveXPlus, MoveXMinus, 
	MoveYPlus, MoveYMinus,
	Rotate
};

UCLASS()
class UE_IOROOMPROJECT_API AFurnitureManipulator : public AActor
{
	GENERATED_BODY()

public:
	AFurnitureManipulator();
	
	void AttachTo(AFurnitureActor* Target);
	
	EManipulatorHandle ResolveHandle(UPrimitiveComponent* Component) const;
	
	void SetHoveredHandle(EManipulatorHandle Handle);

protected:
	virtual void BeginPlay() override;
	
	// components
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArrowXPlus;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArrowXMinus;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArrowYPlus;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArrowYMinus;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> RotationRing;
	
	// properties
	UPROPERTY(EditAnywhere, Category="Manipulator")
	TObjectPtr<UMaterialInterface> BaseMaterial;
	
	UPROPERTY(EditAnywhere, Category="Manipulator")
	TObjectPtr<UMaterialInterface> HoverMaterial;
	
	UPROPERTY(EditAnywhere, Category="Manipulator")
	float HandleMargin = 20.f;
	
	UPROPERTY(EditAnywhere, Category="Manipulator")
	float HandleHeightOffset = 0.f;
	
	UPROPERTY(EditAnywhere, Category="Manipulator")
	float HandleSpaceArrowRing = 10.f;
	
	
	EManipulatorHandle HoveredHandle = EManipulatorHandle::None;
	
private:
	
	void PositionHandle(const FBox& Bounds);
	UStaticMeshComponent* ComponentForHandle(EManipulatorHandle Handle) const;
	

};
