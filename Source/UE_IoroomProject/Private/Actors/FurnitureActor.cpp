// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/FurnitureActor.h"


// Sets default values
AFurnitureActor::AFurnitureActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	FurnitureMesh = CreateDefaultSubobject<UStaticMeshComponent>("FurnitureMesh");
	RootComponent = FurnitureMesh;
}

void AFurnitureActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFurnitureActor::OnHovered()
{
	FurnitureMesh->SetOverlayMaterial(MouseHoverMaterial);
}

void AFurnitureActor::OnUnHovered()
{
	FurnitureMesh->SetOverlayMaterial(nullptr);
}

void AFurnitureActor::OnSelected()
{
	FurnitureMesh->SetRenderCustomDepth(true);
	FurnitureMesh->SetCustomDepthStencilValue(1);
	bIsSelected = true;
}

void AFurnitureActor::OnDeselected()
{
	FurnitureMesh->SetRenderCustomDepth(false);
	bIsSelected = false;
}


