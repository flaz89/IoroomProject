// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DesktopPawn.h"


// Sets default values
ADesktopPawn::ADesktopPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADesktopPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADesktopPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADesktopPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

