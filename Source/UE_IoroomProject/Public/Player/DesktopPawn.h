// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "DesktopPawn.generated.h"

class AFurnitureActor;
class USpringArmComponent;
class UFloatingPawnMovement;
class UInputAction;
class UCameraComponent;

enum class ELMBSate
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
	virtual void Tick(float DeltaTime) override;
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
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UFloatingPawnMovement> FloatingPawnMovement;

	// Input Actions
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> Move;

	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> Look;
	
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> LookActctivate;

	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> Pan;

	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> Zoom;

	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UInputAction> LeftClick;

	// Properties
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float MovementSpeed = 1200.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Pan")
	float PanSpeed = 1200.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Zoom")
	float ZoomSpeed;

	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	float OrbitDragThreshold = 5.f;

	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	float OrbitSensitivity = 0.5f;
	
	// Hovered / Pressed / Selected Actors 
	TObjectPtr<AFurnitureActor> HoveredFurniture;
	TObjectPtr<AFurnitureActor> PressedFurniture;
	TObjectPtr<AFurnitureActor> SelectedFurniture;

private:
	// movement functions
	UFUNCTION(Server, Unreliable)
	void Server_Move(FVector2D AxisValue);
	void ApplyMovement(FVector2D AxisValue);
	void Movement(const FInputActionValue& Value);
	
	// look functions
	UFUNCTION(Server, Unreliable)
	void Server_Look(FVector2D AxisValue);
	void ApplyLook(FVector2D AxisValue);
	void LookAround(const FInputActionValue& Value);
	
	// pan functions
	UFUNCTION(Server, Unreliable)
	void Server_Pan(FVector2D AxisValue);
	void ApplyPan(FVector2D AxisValue);
	void Panning(const FInputActionValue& Value);
	void OnPanStarted();
	void OnPanStopped();
	
	// zoom functions
	UFUNCTION(Server, Unreliable)
	void Server_Zoom(float ZoomFactor);
	void ApplyZoom(float ZoomFactor);
	void Zooming(const FInputActionValue& Value);
	

	UFUNCTION(Server, Unreliable)
	void Server_OrbitTransform(FVector NewLocation, FRotator NewRotation);
	void HandleOrbit(FVector2D CurrentMousePosition);
	void LeftClicking(const FInputActionValue& Value);
	void LeftClickingHeld();
	void LeftClickingReleased();
	
	void HandleDrag();
	
	void OnCameraControlStarted();
	void OnCameraControlStopped();
	
	void UpdateHover(const APlayerController* PlayerController);
	void UpdateCursor(APlayerController* PlayerController);
	
	bool bCameraControlActive = false;
	bool bIsPanning = false;
	
	ELMBSate LeftClickState = ELMBSate::Idle;
	
	FVector2D MousePositionOnClick; // when LMB clicked
	FVector2D LastMousePosition; // set each frame when LMB dragged
	
	// Orbit
	FVector OrbitPivot;
	float OrbitRadius;
	FVector OrbitEntryStartPivot;
	float OrbitEntryAlpha = 0.f;
	
	// Drag
	FVector DragOffset;
	float DragPlaneZ;
};
