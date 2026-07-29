// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "CameraControlComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UE_IOROOMPROJECT_API UCameraControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraControlComponent();
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float MovementSpeed = 1200.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Pan")
	float PanSpeed = 1200.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Zoom")
	float ZoomSpeed;
	
	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	float OrbitSensitivity = 0.5f;
	
	// start functions bound to EnhancedInput in DesktopPawn.cpp 
	void Movement(const FInputActionValue& Value);
	// end functions
	
	
private:
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;
	
	// start movement functions
	void ApplyMovement(FVector2D AxisValue);
	
	UFUNCTION(Server, Unreliable)
	void Server_Move(FVector2D AxisValue);
	// start movement functions
};
