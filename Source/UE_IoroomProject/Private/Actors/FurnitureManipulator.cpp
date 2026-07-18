// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/FurnitureManipulator.h"

#include "Actors/FurnitureActor.h"
#include "Components/StaticMeshComponent.h"


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

void AFurnitureManipulator::AttachTo(AFurnitureActor* Target)
{
	if (Target == nullptr) return;
	AttachToActor(Target, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void AFurnitureManipulator::BeginPlay()
{
	Super::BeginPlay();
	
}



