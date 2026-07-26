// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/IoroomUIFunctionLibrary.h"

#include "Core/Types/IoroomTypes.h"

FText UIoroomUIFunctionLibrary::GridSnapToText(EGridSnap Snap)
{
	switch (Snap)
	{
	case EGridSnap::Minimum: return NSLOCTEXT("SnapSettings", "Grid_Min",    "1");
	case EGridSnap::Fine:    return NSLOCTEXT("SnapSettings", "Grid_Fine",   "5");
	case EGridSnap::Small:   return NSLOCTEXT("SnapSettings", "Grid_Small",  "20");
	case EGridSnap::Medium:  return NSLOCTEXT("SnapSettings", "Grid_Medium", "50");
	case EGridSnap::Large:   return NSLOCTEXT("SnapSettings", "Grid_Large",  "100");
	default:                 return FText::GetEmpty();  
	}
}

FText UIoroomUIFunctionLibrary::RotationSnapToText(ERotationSnap Snap)
{
	switch (Snap) {
	case ERotationSnap::Minimum: return NSLOCTEXT("SnapSettings", "Rotation_Min",    "1");
	case ERotationSnap::Fine:    return NSLOCTEXT("SnapSettings", "Rotation_Fine",   "5");
	case ERotationSnap::Small:   return NSLOCTEXT("SnapSettings", "Rotation_Small",  "15");
	case ERotationSnap::Medium:  return NSLOCTEXT("SnapSettings", "Rotation_Medium", "45");
	case ERotationSnap::Large:   return NSLOCTEXT("SnapSettings", "Rotation_Large",  "90");
	default:					 return FText::GetEmpty();  
	}
}
