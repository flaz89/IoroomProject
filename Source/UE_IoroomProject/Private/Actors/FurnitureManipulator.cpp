// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/FurnitureManipulator.h"

#include "Actors/FurnitureActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"


AFurnitureManipulator::AFurnitureManipulator()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;
	
	ArrowXPlus = CreateDefaultSubobject<UStaticMeshComponent>("ArrowXPlus");
	ArrowXPlus->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ArrowXPlus->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArrowXPlus->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	ArrowXPlus->SetupAttachment(Root);
	
	ArrowXMinus = CreateDefaultSubobject<UStaticMeshComponent>("ArrowXMinus");
	ArrowXMinus->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ArrowXMinus->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArrowXMinus->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	ArrowXMinus->SetupAttachment(Root);
	
	ArrowYPlus = CreateDefaultSubobject<UStaticMeshComponent>("ArrowYPlus");
	ArrowYPlus->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ArrowYPlus->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArrowYPlus->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	ArrowYPlus->SetupAttachment(Root);
	
	ArrowYMinus = CreateDefaultSubobject<UStaticMeshComponent>("ArrowYMinus");
	ArrowYMinus->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ArrowYMinus->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArrowYMinus->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	ArrowYMinus->SetupAttachment(Root);
	
	RotationRing = CreateDefaultSubobject<UStaticMeshComponent>("RotationRing");
	RotationRing->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RotationRing->SetCollisionResponseToAllChannels(ECR_Ignore);
	RotationRing->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
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
 * Determines the type of manipulator handle associated with the provided component.
 * This method identifies the specific handle type (e.g., movement along X or Y axes,
 * or rotation) based on the input component and maps it to its corresponding enum value.
 *
 * @param Component The primitive component to resolve the handle for. Typically, this is
 *                  one of the visual components (arrows or rotation ring) of the manipulator.
 * @return An EManipulatorHandle enum value representing the type of manipulator handle
 *         corresponding to the provided component. Returns EManipulatorHandle::None if
 *         the component does not match any predefined handles.
 */
EManipulatorHandle AFurnitureManipulator::ResolveHandle(UPrimitiveComponent* Component) const
{
	if (Component == ArrowXPlus)  return EManipulatorHandle::MoveXPlus;                                                                                                                                                 
	if (Component == ArrowXMinus) return EManipulatorHandle::MoveXMinus;                                                                                                                                                
	if (Component == ArrowYPlus)  return EManipulatorHandle::MoveYPlus;                                                                                                                                                 
	if (Component == ArrowYMinus) return EManipulatorHandle::MoveYMinus;                                                                                                                                                
	if (Component == RotationRing) return EManipulatorHandle::Rotate;                                                                                                                                                   
	return EManipulatorHandle::None;       
}


/**
 * Retrieves the static mesh component associated with a specific manipulator handle.
 * This method maps an enumerated handle type to its corresponding visual static mesh
 * component, used to represent the handle in the manipulator.
 *
 * @param Handle The EManipulatorHandle value representing the manipulator handle to look up.
 *               Possible values include MoveXPlus, MoveXMinus, MoveYPlus, MoveYMinus, and Rotate.
 * @return A pointer to the UStaticMeshComponent corresponding to the given handle.
 *         Returns nullptr if the handle is not associated with any component.
 */
UStaticMeshComponent* AFurnitureManipulator::ComponentForHandle(EManipulatorHandle Handle) const
{
	switch (Handle)
	{
		case EManipulatorHandle::MoveXPlus: return ArrowXPlus;
		case EManipulatorHandle::MoveXMinus: return ArrowXMinus;
		case EManipulatorHandle::MoveYPlus: return ArrowYPlus;
		case EManipulatorHandle::MoveYMinus: return ArrowYMinus;
		case EManipulatorHandle::Rotate: return RotationRing;
		default: return nullptr;
	}
}

void AFurnitureManipulator::SetHoveredHandle(EManipulatorHandle Handle)
{
	if (Handle == HoveredHandle) return;
	if (UStaticMeshComponent* HandleComponent = ComponentForHandle(HoveredHandle))
	{
		HandleComponent->SetMaterial(0, BaseMaterial);
	}
	if (UStaticMeshComponent* HandleComponent = ComponentForHandle(Handle))
	{
		HandleComponent->SetMaterial(0, HoverMaterial);
	}
	HoveredHandle = Handle;
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





