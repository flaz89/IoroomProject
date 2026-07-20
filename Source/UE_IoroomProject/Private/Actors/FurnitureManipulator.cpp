// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/FurnitureManipulator.h"

#include "Actors/FurnitureActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"


AFurnitureManipulator::AFurnitureManipulator()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;
	
	ArrowXPlus = CreateDefaultSubobject<UStaticMeshComponent>("ArrowXPlus");
	ArrowXPlus->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowXPlus->SetupAttachment(Root);
	
	ArrowXMinus = CreateDefaultSubobject<UStaticMeshComponent>("ArrowXMinus");
	ArrowXMinus->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowXMinus->SetupAttachment(Root);
	
	ArrowYPlus = CreateDefaultSubobject<UStaticMeshComponent>("ArrowYPlus");
	ArrowYPlus->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowYPlus->SetupAttachment(Root);
	
	ArrowYMinus = CreateDefaultSubobject<UStaticMeshComponent>("ArrowYMinus");
	ArrowYMinus->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowYMinus->SetupAttachment(Root);
	
	RotationRing = CreateDefaultSubobject<UStaticMeshComponent>("RotationRing");
	RotationRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RotationRing->SetupAttachment(Root);
}

void AFurnitureManipulator::BeginPlay()
{
	Super::BeginPlay();
}

void AFurnitureManipulator::AttachTo(AFurnitureActor* Target)
{
	if (Target == nullptr) return;
	AttachToActor(Target, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	PositionHandle(Target->GetMeshBoundingBox());
}


/**
 * Positions the handles and rotation ring of the furniture manipulator based on the given bounding box.
 * The method calculates the center and half-extent of the bounding box and adjusts the relative locations
 * of the visual components (arrows and rotation ring) of the manipulator accordingly.
 *
 * @param Bounds The bounding box of the target furniture mesh. It is used to determine the center and
 *               extent for positioning the visual components of the manipulator.
 */
void AFurnitureManipulator::PositionHandle(const FBox& Bounds)
{
	const FVector Center = Bounds.GetCenter();
	const FVector HalfExtent = Bounds.GetExtent();
	const float TargetRadius = FMath::Max(HalfExtent.X, HalfExtent.Y) + HandleMargin;
	const float ArrowZ = - HalfExtent.Z + HandleHeightOffset;
	
	// Arrows position (rotation done in BP)
	ArrowXPlus->SetRelativeLocation(Center + FVector(TargetRadius + HandleSpaceArrowRing, 0.f, ArrowZ));
	ArrowXMinus->SetRelativeLocation(Center + FVector(-(TargetRadius + HandleSpaceArrowRing), 0.f, ArrowZ));
	ArrowYPlus->SetRelativeLocation(Center + FVector(0.f, TargetRadius + HandleSpaceArrowRing , ArrowZ));
	ArrowYMinus->SetRelativeLocation(Center + FVector(0.f, -(TargetRadius + HandleSpaceArrowRing) , ArrowZ));
	
	// Ring Scale and position
	float RingScale = 1.f;
	
	if (UStaticMesh* RingMesh = RotationRing->GetStaticMesh())
	{
		float NativeRadius = RingMesh->GetBoundingBox().GetExtent().X;
		if (NativeRadius > KINDA_SMALL_NUMBER) RingScale = TargetRadius / NativeRadius;
	}
	RotationRing->SetRelativeScale3D(FVector(RingScale, RingScale, 1.f));
	RotationRing->SetRelativeLocation(FVector(Center.X, Center.Y, Center.Z + ArrowZ));
}



