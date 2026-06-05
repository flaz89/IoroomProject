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
	bReplicates = true;
	SetReplicatingMovement(true);

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
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
		UpdateCursor(PlayerController);
	}
	
}

void ADesktopPawn::UpdateHover(const APlayerController* PlayerController)
{
	FHitResult Hit;
	PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	
	// no hovering starts if panning, looking around / Orbiting / Dragging
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

void ADesktopPawn::UpdateCursor(APlayerController* PlayerController)
{
	if (bCameraControlActive)
	{
		PlayerController->bShowMouseCursor = false;
		return;
	} 
	
	PlayerController->bShowMouseCursor = true;
	
	if (LeftClickState == ELMBSate::Dragging || LeftClickState == ELMBSate::Orbiting)
	{
		PlayerController->CurrentMouseCursor = EMouseCursor::GrabHandClosed;
	}
	else if (HoveredFurniture != nullptr)
	{
		PlayerController->CurrentMouseCursor = EMouseCursor::Hand;
	}
	else
	{
		PlayerController->CurrentMouseCursor = EMouseCursor::Default;
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
		EnhancedInputComponent->BindAction(LookActctivate, ETriggerEvent::Started, this, &ADesktopPawn::OnCameraControlStarted);
		EnhancedInputComponent->BindAction(LookActctivate, ETriggerEvent::Completed, this, &ADesktopPawn::OnCameraControlStopped);

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
				
				if (PressedFurniture && PressedFurniture == SelectedFurniture) // drag mode
				{
					FVector RayOrigin;
					FVector RayDirection;
					PlayerController->DeprojectMousePositionToWorld(RayOrigin, RayDirection);
					
					if (FMath::IsNearlyZero(RayDirection.Z)) return;
					
					DragPlaneZ = PressedFurniture->GetActorLocation().Z;
					const float t = (DragPlaneZ - RayOrigin.Z) / RayDirection.Z;
					const FVector WorldPoint = RayOrigin + t * RayDirection;
					DragOffset = FVector(PressedFurniture->GetActorLocation().X - WorldPoint.X, PressedFurniture->GetActorLocation().Y - WorldPoint.Y, 0.0f);
					
					LeftClickState = ELMBSate::Dragging;
				} 
				else // orbit mode
				{
					OrbitRadius = FVector::Dist(GetActorLocation(), OrbitPivot);
					OrbitEntryStartPivot = GetActorLocation() + GetControlRotation().Vector() * OrbitRadius;
					OrbitEntryAlpha = 0.f;
					
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
			HandleDrag();
		}
	}
}

void ADesktopPawn::HandleDrag()
{
	if (SelectedFurniture == nullptr) return;
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		FVector RayOrigin;
		FVector RayDirection;
		PlayerController->DeprojectMousePositionToWorld(RayOrigin, RayDirection);
		if (FMath::IsNearlyZero(RayDirection.Z)) return;

		const float t = (DragPlaneZ - RayOrigin.Z) / RayDirection.Z;
		const FVector WorldPoint = RayOrigin + t * RayDirection;
		
		// -- drag on object with trace on Z
		float NewX = WorldPoint.X + DragOffset.X;
		float NewY = WorldPoint.Y + DragOffset.Y;
		
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(SelectedFurniture);
		
		FHitResult SurfaceHit;
		GetWorld()->LineTraceSingleByChannel(
			SurfaceHit, 
			FVector(NewX, NewY, DragPlaneZ + 500.f),
			FVector(NewX,NewY, -10000.f),
			ECC_Visibility,
			Params
			);
		
		float NewZ = SurfaceHit.bBlockingHit ? SurfaceHit.ImpactPoint.Z : DragPlaneZ;
		// --
		
		SelectedFurniture->SetActorLocation(FVector(NewX, NewY, NewZ));

		const float NewT = (NewZ- RayOrigin.Z) / RayDirection.Z;
		const FVector NewWorldPoint = RayOrigin + NewT * RayDirection;
		DragOffset.X = NewX - NewWorldPoint.X;
		DragOffset.Y = NewY - NewWorldPoint.Y;
		DragPlaneZ = NewZ;
	}
	
}

void ADesktopPawn::HandleOrbit(FVector2D CurrentMousePosition)
{
	const FVector2D MousePositionDelta = CurrentMousePosition - LastMousePosition;
	
	OrbitEntryAlpha = FMath::Min(OrbitEntryAlpha + 1.f / 10.f, 1.f); // 10.f amount of frame needed (+ slow, - fast)
	const FVector EffectivePivot = FMath::Lerp(OrbitEntryStartPivot, OrbitPivot, OrbitEntryAlpha);

	FVector PivotToCamera = GetActorLocation() - EffectivePivot;

	// Yaw: rotation around world Z axis
	const FQuat YawQuat(FVector::UpVector, FMath::DegreesToRadians(MousePositionDelta.X * OrbitSensitivity));
	PivotToCamera = YawQuat.RotateVector(PivotToCamera);

	// Pitch
	const FVector Horizontal = FVector(PivotToCamera.X, PivotToCamera.Y, 0.f);
	const float CurrentElevation = FMath::RadiansToDegrees(FMath::Atan2(PivotToCamera.Z, Horizontal.Size()));
	const float NewElevation = FMath::Clamp(CurrentElevation + MousePositionDelta.Y * OrbitSensitivity, -89.f, 89.f);

	const float NewHorizontalDistance = FMath::Cos(FMath::DegreesToRadians(NewElevation)) * OrbitRadius;
	const float NewVerticalDistance = FMath::Sin(FMath::DegreesToRadians(NewElevation)) * OrbitRadius;
	PivotToCamera = Horizontal.GetSafeNormal() * NewHorizontalDistance + FVector(0.f, 0.f, NewVerticalDistance);

	SetActorLocation(EffectivePivot + PivotToCamera);
	GetController()->SetControlRotation((EffectivePivot - GetActorLocation()).GetSafeNormal().Rotation());
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
		bUseControllerRotationYaw = true;
		bUseControllerRotationPitch = true;
		bCameraControlActive = true;
	}
}

void ADesktopPawn::OnCameraControlStopped()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;
		bCameraControlActive = false;
	}
}



