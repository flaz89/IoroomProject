// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DesktopPawn.h"

#include "EnhancedInputComponent.h"
#include "Actors/FurnitureActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"

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
	
	// if mac value 5.f, if windows 20.f
	ZoomSpeed = 5.f;
	#if PLATFORM_WINDOWS
		ZoomSpeed = 20.f;
	#endif
}

void ADesktopPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult ))
		{
			AFurnitureActor* HitActor = Cast<AFurnitureActor>(HitResult.GetActor());
			
			if (HitActor != HoveredFurniture)
			{
				if (HoveredFurniture != nullptr) HoveredFurniture->OnUnHovered();
				if (HitActor != nullptr && HitActor != SelectedFurniture && !bCameraControlActive) HitActor->OnHovered();
				HoveredFurniture = HitActor;
			}
		} 
		else
		{
			if (HoveredFurniture != nullptr) HoveredFurniture->OnUnHovered();
		}
	}
}

void ADesktopPawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true; // show cursor
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture); // unlock cursor windows limit
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
		EnhancedInputComponent->BindAction(Look, ETriggerEvent::Triggered, this, &ADesktopPawn::LookAround);
		EnhancedInputComponent->BindAction(Pan, ETriggerEvent::Triggered, this, &ADesktopPawn::Panning);
		EnhancedInputComponent->BindAction(Zoom, ETriggerEvent::Triggered, this, &ADesktopPawn::Zooming);
		// left click
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
 * Left click checks if cursor hits something, 
 * if true catchs mouse moves and save mouse initial position,
 * set OritPivot at click location,
 * set LMBState to Pressed (if on mac Alt is pressed LMBState stay Idle)
 * set DesktopPawn pointer SelectedFurniture with actor hit by cursor
 */
void ADesktopPawn::LeftClicking(const FInputActionValue& Value)
{
	if (!Controller) return;
	FHitResult HitResult;
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
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
			
			LMBState = ELMBState::Pressed;
		}
		ClickedFurniture = Cast<AFurnitureActor>(HitResult.GetActor());
		DrawDebugSphere(GetWorld(), OrbitPivot, 16, 16, FColor::Red, false, 2.f);
	}
}

/*
 * based on LMBState always enter on PRESSED,
 * catchs mouse moves and verify if the movement is > then OrbitDragThreshold,
 * if so set the distance between player and OrbitPivot, then set a OrbitVirtualPivot,
 * enable flag bool to AlignOrbit,
 * set LMBState to enter Orbit mode
 * 
 * in ORBITING state catchs mouse moves,
 * if OrbitAligning is true set orbit OrbitAlignAlpha value, if >=1 set OrbitALigning back to false,
 * define and init CurrentPivot then define NextRotation to apply clamped between -89/89,
 * set orbit rotation and player location
 */
void ADesktopPawn::LeftClickingHeld()
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		switch (LMBState) // switch based on LMB State 
		{
			case ELMBState::Pressed:
				{
					float MouseX;
					float MouseY;
					if (PlayerController->GetMousePosition(MouseX, MouseY))
					{
						const FVector2D MouseCurrentPosition = FVector2D(MouseX, MouseY);
						
						if ((MouseCurrentPosition - MouseInitPosition).Size() > OrbitDragThreshold)
						{
							const FVector InitForward = FRotationMatrix(Controller->GetControlRotation()).GetUnitAxis(EAxis::X);
							OrbitArmLength = (GetActorLocation() - OrbitPivot).Size();
							OrbitVirtualPivot = GetActorLocation() + InitForward * OrbitArmLength;
							bOrbitAligning = true;
							OrbitAlignAlpha = 0.f;
							LMBState = ELMBState::Orbiting;
						}
					}
					break;
				}
				
			case ELMBState::Orbiting:
				{
					if (!Controller) return;
					
					bCameraControlActive = true;
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
					
					break;
				}
			default:
				break;
		}
	}
}

void ADesktopPawn::LeftClickingReleased()
{
	if (LMBState == ELMBState::Pressed)
	{
		if (SelectedFurniture) SelectedFurniture->OnDeselected();
		SelectedFurniture = ClickedFurniture;
		if (SelectedFurniture) SelectedFurniture->OnSelected();
	}
	bCameraControlActive = false;
	bOrbitAligning = false;
	LMBState = ELMBState::Idle;
}

