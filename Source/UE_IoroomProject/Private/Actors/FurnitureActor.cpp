// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/FurnitureActor.h"


// Sets default values
AFurnitureActor::AFurnitureActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	SetReplicatingMovement(true);
	SetNetUpdateFrequency(50.f);
	SetMinNetUpdateFrequency(20.f);
	
	FurnitureMesh = CreateDefaultSubobject<UStaticMeshComponent>("FurnitureMesh");
	FurnitureMesh->SetMobility(EComponentMobility::Movable);
	RootComponent = FurnitureMesh;
}

void AFurnitureActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFurnitureActor::OnHovered()
{
	UE_LOG(LogTemp, Log, TEXT("HOVERED"));
	FurnitureMesh->SetOverlayMaterial(MouseHoverMaterial);
}

void AFurnitureActor::OnUnHovered()
{
	FurnitureMesh->SetOverlayMaterial(nullptr);
}

void AFurnitureActor::OnSelected()
{
	bIsSelected = true;
	FurnitureMesh->SetRenderCustomDepth(true);
	FurnitureMesh->SetCustomDepthStencilValue(1);
}

void AFurnitureActor::OnDeselected()
{
	bIsSelected = false;
	FurnitureMesh->SetRenderCustomDepth(false);
}


