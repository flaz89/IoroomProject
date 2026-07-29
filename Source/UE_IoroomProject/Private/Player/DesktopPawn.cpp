// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/DesktopPawn.h"

#include "EnhancedInputComponent.h"
#include "Actors/FurnitureActor.h"
#include "Actors/FurnitureManipulator.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/IoroomPlayerState.h"
#include "Player/Components/CameraControlComponent.h"

ADesktopPawn::ADesktopPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
	
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneRoot;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 0.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	
	Body = CreateDefaultSubobject<UStaticMeshComponent>("Body");
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetupAttachment(RootComponent);

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("FloatingPawnMovement");
	CameraControl = CreateDefaultSubobject<UCameraControlComponent>("CameraControl");
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

void ADesktopPawn::ApplyPlayerVisuals(FLinearColor Color)
{
	if (!CustomBodyMaterial) CustomBodyMaterial = Body->CreateDynamicMaterialInstance(0);
	if (CustomBodyMaterial) CustomBodyMaterial->SetVectorParameterValue(FName("PlayerColor"), Color);
	
	if (!HoverMaterial && HoverBaseMaterial) HoverMaterial = UMaterialInstanceDynamic::Create(HoverBaseMaterial, this);
	if (HoverMaterial) HoverMaterial->SetVectorParameterValue(FName("HoverColor"), Color);
}

void ADesktopPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	AddTickPrerequisiteActor( NewController);
	
	AIoroomPlayerState* IoroomPlayerState = NewController->GetPlayerState<AIoroomPlayerState>();
	if (IoroomPlayerState) ApplyPlayerVisuals(IoroomPlayerState->AssignedColor);
}

void ADesktopPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	AIoroomPlayerState* IoroomPlayerState = GetPlayerState<AIoroomPlayerState>();
	if (IoroomPlayerState && IoroomPlayerState->StencilSlot > 0)  ApplyPlayerVisuals(IoroomPlayerState->AssignedColor);
}

void ADesktopPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!IsLocallyControlled()) return;
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		UpdateHover(PlayerController);
		UpdateCursor(PlayerController);
		UpdateManipulatorHover(PlayerController);
	}
}

void ADesktopPawn::UpdateHover(const APlayerController* PlayerController)
{
	FHitResult Hit;
	PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	
	// no hovering starts if panning, looking around / Orbiting / Dragging
	if (bCameraControlActive || LeftClickState == ELMBState::Dragging || LeftClickState == ELMBState::Orbiting || LeftClickState == ELMBState::Rotating) 
	{
		if (HoveredFurniture) HoveredFurniture -> OnUnHovered();
		HoveredFurniture = nullptr;
		return;
	}
	
	// if someone selects the furniture the player is hovering
	if (HoveredFurniture && !HoveredFurniture->IsSelectableFurniture())                                                                                                                                                     
	{                                                                                                                                                                                                                       
		HoveredFurniture->OnUnHovered();                                                                                                                                                                                    
		HoveredFurniture = nullptr;                                                                                                                                                                                         
	}      

	if (AFurnitureActor* HitActor = Cast<AFurnitureActor>(Hit.GetActor()))
	{
		// A furniture is hoverable only if it isn't locked by another player and isn't our own selection
		if (HitActor != SelectedFurniture && HitActor->IsSelectableFurniture())
		{
			if (HitActor != HoveredFurniture)
			{
				if (HoveredFurniture) HoveredFurniture -> OnUnHovered();
				HitActor -> OnHovered(HoverMaterial);
				HoveredFurniture = HitActor;
			}
		}
		else if (HoveredFurniture) // hit a furniture we can't hover (locked by another / our own selection) -> drop hover so the cursor resets
		{
			HoveredFurniture -> OnUnHovered();
			HoveredFurniture = nullptr;
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
	
	if (LeftClickState == ELMBState::Dragging || LeftClickState == ELMBState::Orbiting || LeftClickState == ELMBState::Rotating)
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

/**
 * Updates the hover state of the furniture manipulator based on the cursor's position.
 * If the camera control is active or the left mouse button is in a dragging or orbiting state,
 * the hover state is cleared. Otherwise, the manipulator's hover handle is set based on the component
 * currently under the cursor.
 *
 * @param PlayerController The player controller used to determine the cursor's position and the component being hovered over.
 */
void ADesktopPawn::UpdateManipulatorHover(const APlayerController* PlayerController)
{
	if (!ActiveManipulator) return;
	if (bCameraControlActive || LeftClickState == ELMBState::Dragging || LeftClickState == ELMBState::Orbiting || LeftClickState == ELMBState::Rotating)
	{
		ActiveManipulator->SetHoveredHandle(EManipulatorHandle::None);
		return;
	}
	
	FHitResult HandleHit;
	if (PlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, HandleHit))
	{
		EManipulatorHandle Handle = ActiveManipulator->ResolveHandle(HandleHit.GetComponent());
		ActiveManipulator->SetHoveredHandle(Handle);
	}
	else
	{
		ActiveManipulator->SetHoveredHandle(EManipulatorHandle::None);
	} 
}

void ADesktopPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(Move, ETriggerEvent::Triggered, CameraControl.Get(), &UCameraControlComponent::Movement);
		EnhancedInputComponent->BindAction(Zoom, ETriggerEvent::Triggered, this, &ADesktopPawn::Zooming);

		// RMB
		EnhancedInputComponent->BindAction(Look, ETriggerEvent::Triggered, this, &ADesktopPawn::LookAround);
		EnhancedInputComponent->BindAction(LookActctivate, ETriggerEvent::Started, this, &ADesktopPawn::OnCameraControlStarted);
		EnhancedInputComponent->BindAction(LookActctivate, ETriggerEvent::Completed, this, &ADesktopPawn::OnCameraControlStopped);

		// MMB (Mac = left alt + LMB)
		EnhancedInputComponent->BindAction(Pan, ETriggerEvent::Triggered, this, &ADesktopPawn::Panning);
		EnhancedInputComponent->BindAction(PanActivate, ETriggerEvent::Started, this, &ADesktopPawn::OnPanStarted);
		EnhancedInputComponent->BindAction(PanActivate, ETriggerEvent::Completed, this, &ADesktopPawn::OnPanStopped);

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

void ADesktopPawn::Server_Look_Implementation(FRotator NewRotation)
{
	if (!Controller) return;
	Controller->SetControlRotation(NewRotation);
	FaceRotation(NewRotation, 0.f);
}

void ADesktopPawn::ApplyLook(const FVector2D AxisValue)
{
	if (!Controller) return;
	
	FRotator NewRotation = Controller->GetControlRotation();
	NewRotation.Yaw += AxisValue.X;
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch - AxisValue.Y, -89.f, 89.f);

	Controller->SetControlRotation(NewRotation);
	FaceRotation(NewRotation, 0.f);
}

void ADesktopPawn::LookAround(const FInputActionValue& Value)
{
	const FVector2D AxisValue = Value.Get<FVector2D>();
	ApplyLook(AxisValue);
	Server_Look(Controller->GetControlRotation());
}

void ADesktopPawn::Server_Zoom_Implementation(const float ZoomFactor)
{
	ApplyZoom(ZoomFactor);
}

void ADesktopPawn::ApplyZoom(const float ZoomFactor)
{
	/*if (!Controller) return;
	const FRotator ControllerRotation = Controller->GetControlRotation();

	const FVector ForwardDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);

	AddActorWorldOffset(ForwardDirection * ZoomFactor * ZoomSpeed);*/
}

void ADesktopPawn::Zooming(const FInputActionValue& Value)
{
	float ZoomFactor = Value.Get<float>();
	
	#if PLATFORM_WINDOWS
		ZoomFactor = -ZoomFactor;
	#endif
	
	ApplyZoom(ZoomFactor);
	Server_Zoom(ZoomFactor);
}

void ADesktopPawn::Server_Pan_Implementation(FVector2D AxisValue)
{
	ApplyPan(AxisValue);
}

void ADesktopPawn::ApplyPan(const FVector2D AxisValue)
{
	/*if (!Controller) return;
	const FRotator ControllerRotation = Controller->GetControlRotation();
	
	const FVector RightDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Y);
	const FVector UpDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Z);
	const FVector NormalizedDirection = FVector(RightDirection * AxisValue.X + UpDirection * AxisValue.Y).GetClampedToMaxSize(1.f);
	
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	AddActorWorldOffset(NormalizedDirection * PanSpeed * DeltaTime, true );*/
}

void ADesktopPawn::Panning(const FInputActionValue& Value)
{
	const FVector2D AxisValue = Value.Get<FVector2D>();

	ApplyPan(AxisValue);
	Server_Pan(AxisValue);
}

void ADesktopPawn::OnPanStarted()
{
	bIsPanning = true;
	bCameraControlActive = true;
	LeftClickState = ELMBState::Idle;
	PressedFurniture = nullptr;
}

void ADesktopPawn::OnPanStopped()
{
	bIsPanning = false;
	bCameraControlActive = false;
}

void ADesktopPawn::LeftClicking(const FInputActionValue& Value)
{
	if (bCameraControlActive) return;
	if (!Controller) return;
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	
	if (PlayerController)
	{
		// left-click on Manipulator
		if (ActiveManipulator && SelectedFurniture)
		{
			FHitResult HandleHit;
			if (PlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, HandleHit))
			{
				const EManipulatorHandle Handle = ActiveManipulator->ResolveHandle(HandleHit.GetComponent());
				// if clicked on the ring
				if (Handle == EManipulatorHandle::Rotate)
				{
					/*
					 * count rotation based on mouse drag after grabbed ring
					 */
					const float PlaneZ = SelectedFurniture->GetActorLocation().Z;
					FVector RayOrigin;
					FVector RayDirection;
					PlayerController->DeprojectMousePositionToWorld(RayOrigin, RayDirection);
					if (FMath::IsNearlyZero(RayDirection.Z)) return;
					
					const float t = (PlaneZ - RayOrigin.Z) / RayDirection.Z;
					const FVector WorldPoint = RayOrigin + t * RayDirection;
					
					const FVector Center = SelectedFurniture->GetActorLocation();
					const float GrabAngle = FMath::RadiansToDegrees(FMath::Atan2(WorldPoint.Y - Center.Y, WorldPoint.X - Center.X));
					
					RotationStartYaw = SelectedFurniture->GetActorRotation().Yaw;
					RotationGrabAngle = GrabAngle;
					RotationPlaneZ = PlaneZ;
					LeftClickState = ELMBState::Rotating;
					return;
				}
				// if clicked on the arrows
				if (NudgeSelectedFurniture(Handle)) return;
			}
		}
		
		// left-click on normal furniture
		FHitResult Hit;
		PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, Hit);
		PressedFurniture = Cast<AFurnitureActor>(Hit.GetActor());
		
		float MouseX;
		float MouseY;
		PlayerController->GetMousePosition(MouseX, MouseY);
		MousePositionOnClick = FVector2D(MouseX, MouseY);
		
		OrbitPivot = Hit.ImpactPoint;
		LeftClickState = ELMBState::Pressed;
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
		
		if (LeftClickState == ELMBState::Pressed) // set in LeftClicking()
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
					
					LeftClickState = ELMBState::Dragging;
					if (ActiveManipulator) ActiveManipulator->SetActorHiddenInGame(true);
				} 
				else // orbit mode
				{
					OrbitRadius = FVector::Dist(GetActorLocation(), OrbitPivot);
					OrbitEntryStartPivot = GetActorLocation() + GetControlRotation().Vector() * OrbitRadius;
					OrbitEntryAlpha = 0.f;
					
					bUseControllerRotationYaw = true;
					bUseControllerRotationPitch = true;
					LeftClickState = ELMBState::Orbiting;
				}
			}
		}
		else if (LeftClickState == ELMBState::Rotating)
		{
			HandleRotate();
		}
		else if (LeftClickState == ELMBState::Orbiting)
		{
			HandleOrbit(CurrentMousePosition);
		}
		else if (LeftClickState == ELMBState::Dragging)
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

		const FVector NewLocation(NewX, NewY, NewZ);
		SelectedFurniture->SetActorLocation(NewLocation);

		// RPC: let the server (authority) move the furniture so replicated movement reaches other clients
		Server_DragFurniture(NewLocation);

		const float NewT = (NewZ- RayOrigin.Z) / RayDirection.Z;
		const FVector NewWorldPoint = RayOrigin + NewT * RayDirection;
		DragOffset.X = NewX - NewWorldPoint.X;
		DragOffset.Y = NewY - NewWorldPoint.Y;
		DragPlaneZ = NewZ;
	}
}

void ADesktopPawn::HandleRotate()
{
	if (SelectedFurniture == nullptr) return;
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		FVector RayOrigin;
		FVector RayDirection;
		PlayerController->DeprojectMousePositionToWorld(RayOrigin, RayDirection);
		if (FMath::IsNearlyZero(RayDirection.Z)) return;

		const float t = (RotationPlaneZ - RayOrigin.Z) / RayDirection.Z;
		const FVector WorldPoint = RayOrigin + t * RayDirection;

		const FVector Center = SelectedFurniture->GetActorLocation();
		const float CurrentAngle = FMath::RadiansToDegrees(FMath::Atan2(WorldPoint.Y - Center.Y, WorldPoint.X - Center.X));
		
		float RotationSnap = 15.f;
		if (const AIoroomPlayerState* PlayerState = GetPlayerState<AIoroomPlayerState>())
		{
			RotationSnap = PlayerState->GetRotStep();
		}

		const float DeltaSnap = FMath::GridSnap(CurrentAngle - RotationGrabAngle, RotationSnap);
		
		FRotator NewRotation = SelectedFurniture->GetActorRotation();
		NewRotation.Yaw = RotationStartYaw + DeltaSnap;
		SelectedFurniture->SetActorRotation(NewRotation);
		Server_RotateFurniture(NewRotation);
	}
}

/**
 * Nudges the currently selected furniture in a specified direction based on the manipulator handle provided.
 * The nudging operation moves the furniture by a grid step size in the given direction.
 *
 * @param Handle The manipulator handle indicating the direction to nudge the selected furniture.
 *               Accepted values include MoveXPlus, MoveXMinus, MoveYPlus, and MoveYMinus.
 * @return Returns true if the operation is successful; false otherwise. The operation fails if no furniture is selected
 *         or if an invalid handle is provided.
 */
bool ADesktopPawn::NudgeSelectedFurniture(EManipulatorHandle Handle)
{
	if (SelectedFurniture == nullptr) return false;
	
	// get directionF
	FVector Direction;
	switch (Handle)
	{
		case EManipulatorHandle::MoveXPlus: Direction = SelectedFurniture->GetActorForwardVector(); break;
		case EManipulatorHandle::MoveXMinus: Direction = -SelectedFurniture->GetActorForwardVector(); break;
		case EManipulatorHandle::MoveYPlus: Direction = SelectedFurniture->GetActorRightVector(); break;
		case EManipulatorHandle::MoveYMinus: Direction = -SelectedFurniture->GetActorRightVector(); break;
		default: return false;
	}
	
	// get step distance
	float GridStep = 20.f;
	if (AIoroomPlayerState* IoroomPlayerState = GetPlayerState<AIoroomPlayerState>())
	{
		GridStep = IoroomPlayerState->GetGridStep();
	}
	
	const FVector NewLocation = SelectedFurniture->GetActorLocation() + Direction * GridStep;
	SelectedFurniture->SetActorLocation(NewLocation);
	Server_DragFurniture(NewLocation);
	return true;
}

void ADesktopPawn::Server_SelectFurniture_Implementation(AFurnitureActor* Furniture)
{
	if (Furniture == nullptr) return;
	if (!Furniture->IsSelectableFurniture()) return;
	if (SelectedFurniture) SelectedFurniture->OnDeselected();
	
	if (AIoroomPlayerState* IoroomPlayerState = GetPlayerState<AIoroomPlayerState>())
	{
		Furniture->OnSelected(IoroomPlayerState->StencilSlot);
		SelectedFurniture = Furniture;
	}
}

void ADesktopPawn::Server_DeselectFurniture_Implementation()
{
	if (SelectedFurniture == nullptr) return;
	SelectedFurniture->OnDeselected();
	SelectedFurniture = nullptr;
}

void ADesktopPawn::Server_OrbitTransform_Implementation(FVector NewLocation, FRotator NewRotation)
{
	if (!Controller) return;

	SetActorLocation(NewLocation);
	Controller->SetControlRotation(NewRotation);
	FaceRotation(NewRotation, 0.f);
}

/*
 * Server-authoritative drag: moves the furniture this pawn currently owns the selection lock on.
 * Uses the server-side SelectedFurniture (set in Server_SelectFurniture) rather than a passed
 * pointer, so a client can only move furniture it has actually selected/locked. Replicated
 * movement (SetReplicatingMovement on AFurnitureActor) then propagates NewLocation to all clients.
 */
void ADesktopPawn::Server_DragFurniture_Implementation(FVector NewLocation)
{
	if (SelectedFurniture == nullptr) return;
	SelectedFurniture->SetActorLocation(NewLocation);
}

void ADesktopPawn::Server_RotateFurniture_Implementation(FRotator NewRotation)
{
	if (SelectedFurniture == nullptr) return;
	SelectedFurniture->SetActorRotation(NewRotation);
}

void ADesktopPawn::HandleOrbit(FVector2D CurrentMousePosition)
{
	/*const FVector2D MousePositionDelta = CurrentMousePosition - LastMousePosition;
	
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

	const FVector NewLocation = EffectivePivot + PivotToCamera;
	const FRotator NewRotation = (EffectivePivot - NewLocation).GetSafeNormal().Rotation();
	
	SetActorLocation(NewLocation);
	GetController()->SetControlRotation(NewRotation);
	LastMousePosition = CurrentMousePosition;
	
	//RPC
	Server_OrbitTransform(NewLocation, NewRotation);*/
	
}

void ADesktopPawn::LeftClickingReleased()
{
	if (LeftClickState == ELMBState::Pressed)
	{
		if (PressedFurniture)
		{
			Server_SelectFurniture(PressedFurniture);
			PressedFurniture->OnUnHovered();
			SelectedFurniture = PressedFurniture;
			
			// if manipulator already exists on another furniture, destroy that one and spawn on the new selected furniture
			if (ActiveManipulator)
			{
				ActiveManipulator->Destroy();
				ActiveManipulator = nullptr;
			}
			if (FurnitureManipulatorClass)
			{
				ActiveManipulator = GetWorld()->SpawnActor<AFurnitureManipulator>(FurnitureManipulatorClass);
				if (ActiveManipulator) ActiveManipulator->AttachTo(PressedFurniture);
			}
			
		}
		else
		{
			// deselect furniture, remove overlay, remove manipulator
			if (SelectedFurniture) Server_DeselectFurniture();
			SelectedFurniture = nullptr;
			if (ActiveManipulator)
			{
				ActiveManipulator->Destroy();
				ActiveManipulator = nullptr;
			}
		}
	}
	
	if (LeftClickState == ELMBState::Dragging)
	{
		if (ActiveManipulator) ActiveManipulator->SetActorHiddenInGame(false);
	}
	
	PressedFurniture = nullptr;
	LeftClickState = ELMBState::Idle;
}

void ADesktopPawn::OnCameraControlStarted()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		bUseControllerRotationYaw = true;
		bCameraControlActive = true;
	}
}

void ADesktopPawn::OnCameraControlStopped()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		bCameraControlActive = false;
	}
}



