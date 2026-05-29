// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h" // a reference value need the #include
#include "GameFramework/Pawn.h"
#include "DesktopPawn.generated.h"

class AFurnitureActor;
class USpringArmComponent;
class UFloatingPawnMovement;
class UInputAction;
class UCameraComponent;

// Enum left mouse button click state
enum class ELMBState : uint8
{
	Idle,
	Pressed,
	Orbiting,
	Dragging
};

UCLASS()
class UE_IOROOMPROJECT_API ADesktopPawn : public APawn
{
	GENERATED_BODY()

public:
	ADesktopPawn();
	void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	
	// Components
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UFloatingPawnMovement> FloatingPawnMovement;
	
	// Input Actions
	
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> Move;
	
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> Look;
	
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> Pan;
	
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> Zoom;
	
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> LeftClick;
	
	// Properties
	UPROPERTY(EditDefaultsOnly, Category="Zoom")
	float ZoomSpeed;
	
	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	float OrbitDragThreshold = 5.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	float OrbitSensitivity = 1.f;
	
private:
	
	// functions bound to Input Actions
	void Movement(const FInputActionValue& Value);
	void LookAround(const FInputActionValue& Value);
	void Panning(const FInputActionValue& Value);
	void Zooming(const FInputActionValue& Value);
	void LeftClicking(const FInputActionValue& Value);
	void LeftClickingHeld();
	void LeftClickingReleased();
	void OnCameraControlStarted();
	void OnCameraControlStopped();
	
	// orbit properties
	FVector OrbitPivot;
	FVector OrbitVirtualPivot;
	float OrbitArmLength;
	FVector2D MouseInitPosition;
	bool bOrbitAligning = false;
	float OrbitAlignAlpha;
	bool bCameraControlActive = false;
	
	// drag properties
	float DragPlaneZ;
	FVector DragOffset;
	
	// Furniture actors
	TObjectPtr<AFurnitureActor> HoveredFurniture;
	TObjectPtr<AFurnitureActor> ClickedFurniture;
	TObjectPtr<AFurnitureActor> SelectedFurniture;
	
	ELMBState LMBState = ELMBState::Idle;
};
