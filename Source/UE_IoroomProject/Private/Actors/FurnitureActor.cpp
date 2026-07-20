// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/FurnitureActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AFurnitureActor::AFurnitureActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);
	SetNetUpdateFrequency(50.f);
	SetMinNetUpdateFrequency(20.f);
	
	FurnitureMesh = CreateDefaultSubobject<UStaticMeshComponent>("FurnitureMesh");
	FurnitureMesh->SetMobility(EComponentMobility::Movable);
	RootComponent = FurnitureMesh;
}

/*
 * Registers SelectingStencilSlot for replication. The ReplicatedUsing callback
 * drives all visual state changes so no additional properties need replication.
 */
void AFurnitureActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFurnitureActor, SelectingStencilSlot)
}

void AFurnitureActor::BeginPlay()
{
	Super::BeginPlay();

}

/*
 * Fires on clients when SelectingStencilSlot replicates, and is called manually on the server
 * after each assignment (RepNotify callbacks do not fire on the authority).
 * Slot > 0 means selected: enables custom depth and writes the player's stencil slot so the
 * PPV material can resolve the correct outline color.
 */
void AFurnitureActor::OnRep_SelectingStencilSlot()
{
	bool bSelected = SelectingStencilSlot > 0;
	FurnitureMesh->SetRenderCustomDepth(bSelected);
	if (bSelected) FurnitureMesh->SetCustomDepthStencilValue(SelectingStencilSlot);
}

void AFurnitureActor::OnHovered(UMaterialInterface* OverlayMaterial)
{
	FurnitureMesh->SetOverlayMaterial(OverlayMaterial);
}

void AFurnitureActor::OnUnHovered()
{
	FurnitureMesh->SetOverlayMaterial(nullptr);
}

/*
 * Server-only: locks this furniture to the given player stencil slot.
 * The caller is responsible for checking IsSelectableFurniture() before calling.
 */
void AFurnitureActor::OnSelected(int32 InStencilSlot)
{
	if (!HasAuthority()) return;
	SelectingStencilSlot = InStencilSlot;
	OnRep_SelectingStencilSlot();
}

/*
 * Server-only: releases the selection lock, making the furniture available again.
 */
void AFurnitureActor::OnDeselected()
{
	if (!HasAuthority()) return;
	SelectingStencilSlot = 0;
	OnRep_SelectingStencilSlot();
}

FBox AFurnitureActor::GetMeshBoundingBox() const
{
	const UStaticMesh* Mesh = FurnitureMesh->GetStaticMesh();
	if (!Mesh) return FBox();

	const FBox MeshBoundingBox = Mesh->GetBoundingBox();
	const FVector ActorScale = GetActorScale3D();
	
	// return bounding box scaled by the actor scale
	return FBox(MeshBoundingBox.Min * ActorScale, MeshBoundingBox.Max * ActorScale);
}


