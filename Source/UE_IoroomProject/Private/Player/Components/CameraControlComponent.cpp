// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Components/CameraControlComponent.h"

#include "GameFramework/Pawn.h"


UCameraControlComponent::UCameraControlComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault( true);
}


void UCameraControlComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPawn = Cast<APawn>(GetOwner());
}


// Called every frame
void UCameraControlComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

