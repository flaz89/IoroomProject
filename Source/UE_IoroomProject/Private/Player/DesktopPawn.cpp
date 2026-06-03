// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/DesktopPawn.h"

#include "EnhancedInputComponent.h"
#include "Actors/FurnitureActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"

ADesktopPawn::ADesktopPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneRoot;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 0.f;
	SpringArm->bUsePawnControlRotation = true;
	//SpringArm->bEnableCameraRotationLag = false;
	//SpringArm->CameraRotationLagSpeed = 5.f;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	
	Body = CreateDefaultSubobject<UStaticMeshComponent>("Body");
	Body->SetupAttachment(RootComponent);

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("FloatingPawnMovement");

	ZoomSpeed = 5.f;
	#if PLATFORM_WINDOWS
		ZoomSpeed = 20.f;
	#endif
}

void ADesktopPawn::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}
}

void ADesktopPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UpdateHover(PlayerController);
	}
	
}

void ADesktopPawn::UpdateHover(const APlayerController* PlayerController)
{
	FHitResult Hit;
	PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	
	// no hovering starts if panning or looking around 
	if (bCameraControlActive || LeftClickState == ELMBSate::Dragging || LeftClickState == ELMBSate::Orbiting) 
	{
		if (HoveredFurniture) HoveredFurniture -> OnUnHovered();
		HoveredFurniture = nullptr;
		return;
	}

	if (AFurnitureActor* HitActor = Cast<AFurnitureActor>(Hit.GetActor()))
	{
		if (HitActor != HoveredFurniture && HitActor != SelectedFurniture)
		{
			if (HoveredFurniture) HoveredFurniture -> OnUnHovered();
			HitActor -> OnHovered();
			HoveredFurniture = HitActor;
		}
	}
	else
	{
		if (HoveredFurniture) HoveredFurniture -> OnUnHovered();
		HoveredFurniture = nullptr;
	}
}

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
		EnhancedInputComponent->BindAction(Pan, ETriggerEvent::Started, this, &ADesktopPawn::OnPanStarted);
		EnhancedInputComponent->BindAction(Pan, ETriggerEvent::Completed, this, &ADesktopPawn::OnPanStopped);

		// LMB
		EnhancedInputComponent->BindAction(LeftClick, ETriggerEvent::Started, this, &ADesktopPawn::LeftClicking);
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
	const FRotator ControllerRotation = Controller->GetControlRotation();

	const FVector ForwardDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, AxisValue.Y);
	AddMovementInput(RightDirection, AxisValue.X);
}

void ADesktopPawn::LookAround(const FInputActionValue& Value)
{
	if (!Controller) return;
	const FVector2D AxisValue = Value.Get<FVector2D>();

	AddControllerYawInput(AxisValue.X);
	AddControllerPitchInput(AxisValue.Y);
}

void ADesktopPawn::Zooming(const FInputActionValue& Value)
{
	if (!Controller) return;
	float ZoomFactor = Value.Get<float>();
	const FRotator ControllerRotation = Controller->GetControlRotation();

	const FVector ForwardDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);

#if PLATFORM_WINDOWS
	ZoomFactor = -ZoomFactor;
#endif

	AddActorWorldOffset(ForwardDirection * ZoomFactor * ZoomSpeed);
}

void ADesktopPawn::Panning(const FInputActionValue& Value)
{
	if (!Controller) return;
	const FVector2D AxisValue = Value.Get<FVector2D>();

	const FRotator ControllerRotation = Controller->GetControlRotation();
	const FVector RightDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Y);
	const FVector UpDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Z);

	AddMovementInput(RightDirection, AxisValue.X);
	AddMovementInput(UpDirection, AxisValue.Y);
}

void ADesktopPawn::OnPanStarted()
{
	bIsPanning = true;
	bCameraControlActive = true;
	LeftClickState = ELMBSate::Idle;
	PressedFurniture = nullptr;
}

void ADesktopPawn::OnPanStopped()
{
	bIsPanning = false;
	bCameraControlActive = false;
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->bShowMouseCursor = true;
	}
}

void ADesktopPawn::LeftClicking(const FInputActionValue& Value)
{
	if (!Controller) return;
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	
	if (PlayerController)
	{
		FHitResult Hit;
		PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, Hit);
		PressedFurniture = Cast<AFurnitureActor>(Hit.GetActor());
		
		float MouseX;
		float MouseY;
		PlayerController->GetMousePosition(MouseX, MouseY);
		MousePositionOnClick = FVector2D(MouseX, MouseY);
		
		OrbitPivot = Hit.ImpactPoint;
		LeftClickState = ELMBSate::Pressed;
		DrawDebugSphere(GetWorld(), OrbitPivot, 10.f, 10, FColor::Red, false, 1.5f);
	}
}


/*
 * This function is on Event::Trigger so each frame holding Left Mouse Button (LMB) pressed checks if PlayerController exists and 
 * stores the current mouse position on screen, after it checks the state of LMB to chooses if execute Drag or Orbit.
 */
void ADesktopPawn::LeftClickingHeld()
{
	if (!Controller) return;
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	
	if (PlayerController)
	{
		float CurrentMouseX;
		float CurrentMouseY;
		PlayerController->GetMousePosition(CurrentMouseX, CurrentMouseY);
		const FVector2D CurrentMousePosition = FVector2D(CurrentMouseX, CurrentMouseY);
		
		if (LeftClickState == ELMBSate::Pressed) // set in LeftClicking()
		{
			if ((CurrentMousePosition - MousePositionOnClick).Size() > OrbitDragThreshold) // if player is dragging
			{
				LastMousePosition = CurrentMousePosition;
				OrbitRadius = FVector::Dist(GetActorLocation(), OrbitPivot);
				
				if (PressedFurniture && PressedFurniture == SelectedFurniture)
				{
					LeftClickState = ELMBSate::Dragging;
				} 
				else
				{
					bUseControllerRotationYaw = true;
					bUseControllerRotationPitch = true;
					LeftClickState = ELMBSate::Orbiting;
				}
			}
		}
		else if (LeftClickState == ELMBSate::Orbiting)
		{
			HandleOrbit(CurrentMousePosition);
		}
		else if (LeftClickState == ELMBSate::Dragging)
		{
			HandleDrag(CurrentMousePosition);
		}
	}
}

void ADesktopPawn::HandleDrag(FVector2D CurrentMousePosition)
{
	
}

void ADesktopPawn::HandleOrbit(FVector2D CurrentMousePosition)
{
	FVector2D MousePositionDelta = CurrentMousePosition - LastMousePosition;
	
	FRotator Rotation = GetControlRotation();
	Rotation.Yaw += MousePositionDelta.X * OrbitSensitivity;
	Rotation.Pitch = FMath::Clamp(Rotation.Pitch - MousePositionDelta.Y * OrbitSensitivity, -89.f, 89.f);
	GetController()->SetControlRotation(Rotation);

	SetActorLocation(OrbitPivot - GetControlRotation().Vector() * OrbitRadius);
	LastMousePosition = CurrentMousePosition;
}

void ADesktopPawn::LeftClickingReleased()
{
	
	if (LeftClickState == ELMBSate::Pressed)
	{
		if (PressedFurniture)
		{
			if (SelectedFurniture) SelectedFurniture->OnDeselected();
			PressedFurniture->OnSelected();
			PressedFurniture->OnUnHovered();
			SelectedFurniture = PressedFurniture;
		}
		else
		{
			if (SelectedFurniture) SelectedFurniture->OnDeselected();
			SelectedFurniture = nullptr;
		}
	}
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	PressedFurniture = nullptr;
	LeftClickState = ELMBSate::Idle;
}

void ADesktopPawn::OnCameraControlStarted()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->bShowMouseCursor = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationPitch = true;
		bCameraControlActive = true;
	}
}

void ADesktopPawn::OnCameraControlStopped()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->bShowMouseCursor = true;
		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;
		bCameraControlActive = false;
	}
}



