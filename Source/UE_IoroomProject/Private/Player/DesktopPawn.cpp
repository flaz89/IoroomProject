// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DesktopPawn.h"

#include "EnhancedInputComponent.h"
#include "Actors/FurnitureActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

/*
 * CONSTRUCTOR
 * - set root component
 * - set sprigarm and its bheaviour
 * - set camera
 * - set floating pawn movement component
 * - chooses ZoomSpeed value based on OS
 */
ADesktopPawn::ADesktopPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneRoot;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 0.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 5.f;
	
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("FloatingPawnMovement");
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>("PhysicsHandle");
	
	// if mac value 5.f, if windows 20.f
	ZoomSpeed = 5.f;
	#if PLATFORM_WINDOWS
		ZoomSpeed = 20.f;
	#endif
}

void ADesktopPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UpdateHover(PlayerController);
		UpdateDrag(PlayerController);
		UpdateCursor(PlayerController);
	}
}

void ADesktopPawn::UpdateHover(APlayerController* PC)
{
	FHitResult HitResult;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult ))
	{
		AFurnitureActor* HitActor = Cast<AFurnitureActor>(HitResult.GetActor());

		if (HitActor != HoveredFurniture)
		{
			if (HoveredFurniture != nullptr) HoveredFurniture->OnUnHovered();
			if (HitActor != nullptr && HitActor != SelectedFurniture && !bCameraControlActive)
			{
				PC->CurrentMouseCursor = EMouseCursor::Hand;
				HitActor->OnHovered();
			} 
			else
			{
				PC->CurrentMouseCursor = EMouseCursor::Default;
			}
			HoveredFurniture = HitActor;
		}
	}
	else
	{
		if (HoveredFurniture != nullptr) HoveredFurniture->OnUnHovered();
	}
}

void ADesktopPawn::UpdateDrag(APlayerController* PC)
{
	if (LMBState == ELMBState::Dragging)
	{
		//const FVector2D VirtualMousePos = MouseInitPosition + FVector2D(AccumulatedDragDelta.X, -AccumulatedDragDelta.Y);
		float MouseX;
		float MouseY;
		PC->GetMousePosition(MouseX, MouseY);
		FVector WorldLocation;
		FVector WorldDirection;
		PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);
		const float t = (DragPlaneZ - WorldLocation.Z) / WorldDirection.Z;
		PhysicsHandle->SetTargetLocation(WorldLocation + t * WorldDirection);
	}
}

void ADesktopPawn::UpdateCursor(APlayerController* PC)
{
	if (LMBState == ELMBState::Dragging)
		PC->CurrentMouseCursor = EMouseCursor::GrabHandClosed;
	else if (HoveredFurniture != nullptr)
		PC->CurrentMouseCursor = EMouseCursor::Hand;
	else
		PC->CurrentMouseCursor = EMouseCursor::Default;
}

void ADesktopPawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true; // show cursor
		FInputModeGameAndUI InputMode;
		//InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture); // unlock cursor windows limit
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}
}

// functions Input and bound to Input Actions
void ADesktopPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(Move, ETriggerEvent::Triggered, this, &ADesktopPawn::Movement);
		EnhancedInputComponent->BindAction(Zoom, ETriggerEvent::Triggered, this, &ADesktopPawn::Zooming);
		
		// RMB
		EnhancedInputComponent->BindAction(Look, ETriggerEvent::Triggered, this, &ADesktopPawn::LookAround);
		EnhancedInputComponent->BindAction(Look, ETriggerEvent::Started, this, &ADesktopPawn::OnCameraControlStarted);
		EnhancedInputComponent->BindAction(Look, ETriggerEvent::Completed, this, &ADesktopPawn::OnCameraControlStopped);
		
		// MMB (Mac = left alt + LMB)
		EnhancedInputComponent->BindAction(Pan, ETriggerEvent::Triggered, this, &ADesktopPawn::Panning);
		EnhancedInputComponent->BindAction(Pan, ETriggerEvent::Started, this, &ADesktopPawn::OnCameraControlStarted);
		EnhancedInputComponent->BindAction(Pan, ETriggerEvent::Completed, this, &ADesktopPawn::OnCameraControlStopped);
		
		// LMB
		EnhancedInputComponent->BindAction(LeftClick,ETriggerEvent::Started, this, &ADesktopPawn::LeftClicking);
		EnhancedInputComponent->BindAction(LeftClick, ETriggerEvent::Triggered, this, &ADesktopPawn::LeftClickingHeld);
		EnhancedInputComponent->BindAction(LeftClick, ETriggerEvent::Completed, this, &ADesktopPawn::LeftClickingReleased);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DesktopPawn / SetupPlayerInputComponent() -> No input component found"));
	}
}

void ADesktopPawn::Movement(const FInputActionValue& Value)
{
	if (!Controller) return;
	const FVector2D AxisValue = Value.Get<FVector2D>();
	const FRotator ControllerRotaton = Controller->GetControlRotation();
	
	// transform radiant value to vector value wit 4x4 matrix
	const FVector ForwardDirection = FRotationMatrix(ControllerRotaton).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControllerRotaton).GetUnitAxis(EAxis::Y);
	
	AddMovementInput(ForwardDirection, AxisValue.Y);
	AddMovementInput(RightDirection, AxisValue.X);
}

void ADesktopPawn::LookAround(const FInputActionValue& Value)
{
	if (!Controller) return;
	bCameraControlActive = true;
	const FVector2D AxisValue = Value.Get<FVector2D>();

	AddControllerYawInput(AxisValue.X);
	AddControllerPitchInput(AxisValue.Y);
}

void ADesktopPawn::Panning(const FInputActionValue& Value)
{
	if (!Controller) return;
	bCameraControlActive = true;
	const FVector2D AxisValue = Value.Get<FVector2D>();
	
	const FRotator ControllerRotation = Controller->GetControlRotation();
	const FVector RightDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Y);
	const FVector UpDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Z);
	
	AddMovementInput(RightDirection, AxisValue.X);
	AddMovementInput(UpDirection, AxisValue.Y);
}

void ADesktopPawn::Zooming(const FInputActionValue& Value)
{
	if (!Controller) return;
	float ZoomFactor = Value.Get<float>();
	const FRotator ControllerRotation = Controller->GetControlRotation();
	
	const FVector ForwardDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);
	
	// negate windows value for scroll speed for correct movement
	#if PLATFORM_WINDOWS
	ZoomFactor = -ZoomFactor;
	#endif
	
	AddActorWorldOffset(ForwardDirection * ZoomFactor * ZoomSpeed);
}

/*
 * Left Click Mouse Button functions:
 * - LeftClicking: first click and choose which LMBState set (Event STARTED)
 * - LeftClickingHeld: manage which function trigger based oin LMBState (Event TRIGGERED)
 * - LeftClickingReleased set back LMBState to Idle and reset variables (Event COMPLETED)
 */
void ADesktopPawn::LeftClicking(const FInputActionValue& Value)
{
	if (!Controller) return;
	FHitResult HitResult;
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		float MouseX;
		float MouseY;
		if (PlayerController->GetMousePosition(MouseX, MouseY))
		{
			OrbitPivot = HitResult.ImpactPoint; 
			MouseInitPosition = FVector2D(MouseX, MouseY);
					
			#if PLATFORM_MAC
					if (PlayerController->IsInputKeyDown(EKeys::LeftAlt)) return; // on mac if alt for panning is pressed do nothing
			#endif
			
			if (SelectedFurniture && HitResult.GetActor() == SelectedFurniture)
			{
				LMBState = ELMBState::Dragging;
				DragPlaneZ = SelectedFurniture->GetActorLocation().Z;
				AccumulatedDragDelta = FVector2D::ZeroVector;
				
				FVector WorldLocation;
				FVector WorldDirection;
				PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);
				float t = (DragPlaneZ - WorldLocation.Z) / WorldDirection.Z;
				const FVector GrabLocation = WorldLocation + t * WorldDirection;
				
				//DragOffset = SelectedFurniture->GetActorLocation() - (WorldLocation + t * WorldDirection);
				PhysicsHandle->GrabComponentAtLocation(SelectedFurniture->GetFurnitureMesh(), NAME_None, GrabLocation);
			}
			else
			{
				LMBState = ELMBState::Pressed;
				AccumulatedDragDelta = FVector2D::ZeroVector;
			}
		}
		ClickedFurniture = Cast<AFurnitureActor>(HitResult.GetActor());
		DrawDebugSphere(GetWorld(), OrbitPivot, 16, 16, FColor::Red, false, 2.f);
	}
}

void ADesktopPawn::LeftClickingHeld()
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController) return ;
	
	switch (LMBState)
	{
		case ELMBState::Pressed: HandlePressed(PlayerController); break;
		case ELMBState::Orbiting: HandleOrbiting(PlayerController); break;
		case ELMBState::Dragging: HandleDragging(PlayerController); break;
		default: break;
	}
}

void ADesktopPawn::HandlePressed( APlayerController* PlayerController)
{
	float DeltaX;
	float DeltaY;
	PlayerController->GetInputMouseDelta(DeltaX, DeltaY);
	AccumulatedDragDelta += FVector2D(DeltaX, DeltaY);

	if (AccumulatedDragDelta.Size() > OrbitDragThreshold)
	{
		const FVector InitForward = FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::X);
		OrbitArmLength = (GetActorLocation() - OrbitPivot).Size();
		OrbitVirtualPivot = GetActorLocation() + InitForward * OrbitArmLength;
		bOrbitAligning = true;
		OrbitAlignAlpha = 0.f;
		LMBState = ELMBState::Orbiting;
	}
}

void ADesktopPawn::HandleOrbiting( APlayerController* PlayerController)
{
	if (!Controller) return;
					
	const FRotator CurrentRotation = Controller->GetControlRotation();
					
	float DeltaX;
	float DeltaY;
	PlayerController->GetInputMouseDelta(DeltaX, DeltaY);

	if (bOrbitAligning)
	{
		OrbitAlignAlpha = FMath::Min(OrbitAlignAlpha + GetWorld()->GetDeltaSeconds() * 4.f, 1.f); //4.f set blending velocity
		if (OrbitAlignAlpha >= 1.f) bOrbitAligning = false;
	}
					
	const FVector CurrentPivot = bOrbitAligning ? FMath::Lerp(OrbitVirtualPivot, OrbitPivot, FMath::InterpEaseOut(0.f, 1.f, OrbitAlignAlpha, 3.f)) : OrbitPivot;
					
	const FRotator NextRotation(
		FMath::Clamp(CurrentRotation.Pitch + DeltaY * OrbitSensitivity, -89.f, 89.f),
		CurrentRotation.Yaw + DeltaX * OrbitSensitivity,
		0.f
	);
					
	Controller->SetControlRotation(NextRotation);
	SetActorLocation(CurrentPivot - FRotationMatrix(NextRotation).GetUnitAxis(EAxis::X) * OrbitArmLength);
}

void ADesktopPawn::HandleDragging( APlayerController* PlayerController)
{
	float DeltaX;
	float DeltaY;
	PlayerController->GetInputMouseDelta(DeltaX, DeltaY);
	AccumulatedDragDelta += FVector2D(DeltaX, DeltaY);
}

void ADesktopPawn::LeftClickingReleased()
{
	if (LMBState == ELMBState::Pressed)
	{
		if (SelectedFurniture) SelectedFurniture->OnDeselected();
		SelectedFurniture = ClickedFurniture;
		if (SelectedFurniture) SelectedFurniture->OnSelected();
	}
	if (LMBState == ELMBState::Dragging)
	{
		PhysicsHandle->ReleaseComponent();
	}
	bOrbitAligning = false;
	LMBState = ELMBState::Idle;
}



void ADesktopPawn::OnCameraControlStarted()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->bShowMouseCursor = false;
		bCameraControlActive = true;
	}
}

void ADesktopPawn::OnCameraControlStopped()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->bShowMouseCursor = true;
		bCameraControlActive = false;
	}	
}

