// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Components/CameraControlComponent.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"


UCameraControlComponent::UCameraControlComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault( true);
	
	ZoomSpeed = 5.f;
	#if PLATFORM_WINDOWS
	ZoomSpeed = 20.f;
	#endif
}


void UCameraControlComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPawn = Cast<APawn>(GetOwner());
}

// -------------------------------------------------------------------------------------------------------------------- M O V E M E N T
/**
 * Handles camera movement input by processing the movement axis value, applying local movement,
 * and synchronizing the movement with the server.
 *
 * @param Value The input action value representing the movement axis as a 2D vector.
 */
void UCameraControlComponent::Movement(const FInputActionValue& Value)
{
	const FVector2D AxisValue = Value.Get<FVector2D>();
	ApplyMovement(AxisValue);
	Server_Move(AxisValue);
}

/**
 * Processes the input axis value to calculate and apply movement to the owning pawn.
 * The movement is determined based on the controller's rotation, normalizing the direction,
 * and applying a movement offset scaled by the movement speed and frame time.
 *
 * @param AxisValue A 2D vector representing the input axis value for movement along the forward and right directions.
 */
void UCameraControlComponent::ApplyMovement(FVector2D AxisValue)
{
	if (!OwnerPawn) return;
	AController* Controller = OwnerPawn->GetController();
	if (!Controller) return;                                                                                                                                                                                                  
	
	const FRotator ControllerRotation = Controller->GetControlRotation();

	const FVector ForwardDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Y);
	const FVector NormalizedDirection = FVector(ForwardDirection * AxisValue.Y + RightDirection * AxisValue.X).GetClampedToMaxSize(1.f);
	
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	OwnerPawn->AddActorWorldOffset(NormalizedDirection * MovementSpeed * DeltaTime, true);
}

/**
 * Handles the server-side movement logic by forwarding the input axis value to be processed for movement.
 *
 * @param AxisValue The 2D vector representing the movement input axis.
 */
void UCameraControlComponent::Server_Move_Implementation(FVector2D AxisValue)
{
	ApplyMovement(AxisValue);
}

// -------------------------------------------------------------------------------------------------------------------- Z O O M
/**
 * Adjusts the camera zoom level based on input action value and synchronizes the zoom adjustment with the server.
 *
 * @param Value The input action value representing the zoom factor as a float.
 */
void UCameraControlComponent::Zooming(const FInputActionValue& Value)
{
	float ZoomFactor = Value.Get<float>();
	
	#if PLATFORM_WINDOWS
	ZoomFactor = -ZoomFactor;
	#endif
	
	ApplyZoom(ZoomFactor);
	Server_Zoom(ZoomFactor);
}

/**
 * Adjusts the camera's position by applying a zoom operation based on the provided zoom factor.
 *
 * @param ZoomFactor A float value representing the magnitude and direction of the zoom operation.
 *                   A positive value moves the camera forward, while a negative value moves the camera backward.
 */
void UCameraControlComponent::ApplyZoom(float ZoomFactor)
{
	if (!OwnerPawn) return;
	AController* Controller = OwnerPawn->GetController();
	if (!Controller) return;
	const FRotator ControllerRotation = Controller->GetControlRotation();

	const FVector ForwardDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);

	OwnerPawn->AddActorWorldOffset(ForwardDirection * ZoomFactor * ZoomSpeed);
}

/**
 * Executes the server-side zoom functionality by applying a zoom factor to adjust the camera's position.
 *
 * @param ZoomFactor The input value determining the scaling factor for zooming. Positive values zoom in,
 *        while negative values zoom out.
 */
void UCameraControlComponent::Server_Zoom_Implementation(float ZoomFactor)
{
	ApplyZoom(ZoomFactor);
}
