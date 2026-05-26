// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h" // a reference value need the #include
#include "GameFramework/Pawn.h"
#include "DesktopPawn.generated.h"

class USpringArmComponent;
class UFloatingPawnMovement;
class UInputAction;
class UCameraComponent;

UCLASS()
class UE_IOROOMPROJECT_API ADesktopPawn : public APawn
{
	GENERATED_BODY()

public:
	ADesktopPawn();
	
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
	
	// Properties
	UPROPERTY(EditDefaultsOnly, Category="Zoom")
	float ZoomSpeed;
	
private:
	
	// functions bound to Input Actions
	void Movement(const FInputActionValue& Value);
	void LookAround(const FInputActionValue& Value);
	void Panning(const FInputActionValue& Value);
	void Zooming(const FInputActionValue& Value);
};
