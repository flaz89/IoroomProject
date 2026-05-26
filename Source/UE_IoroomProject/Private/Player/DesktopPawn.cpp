// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DesktopPawn.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"


ADesktopPawn::ADesktopPawn()
{
	PrimaryActorTick.bCanEverTick = false;
	
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
	
	ZoomSpeed = 1.f;
#if PLATFORM_WINDOWS
	ZoomSpeed = 15.f
#endif
}

void ADesktopPawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
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
	const FVector2D AxisValue = Value.Get<FVector2D>();

	AddControllerYawInput(AxisValue.X);
	AddControllerPitchInput(AxisValue.Y);
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

void ADesktopPawn::Zooming(const FInputActionValue& Value)
{
	if (!Controller) return;
	const float ZoomFactor = Value.Get<float>();
	const FRotator ControllerRotation = Controller->GetControlRotation();
	
	const FVector ForwardDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);
	
	AddMovementInput(ForwardDirection, ZoomFactor * ZoomSpeed);
}

