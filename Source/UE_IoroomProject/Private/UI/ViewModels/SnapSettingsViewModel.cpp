// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModels/SnapSettingsViewModel.h"

/**
 * Sets the grid snapping level to the specified value.
 *
 * This method updates the grid snapping level used in the application.
 * The snapping level determines the granularity of the grid for snapping operations.
 *
 * @param NewGridSnap The new grid snapping level to set.
 *                    It must be a valid value of the EGridSnap enumeration.
 */
void USnapSettingsViewModel::SetGridSnap(EGridSnap NewGridSnap)
{
	UE_MVVM_SET_PROPERTY_VALUE(GridSnap, NewGridSnap);
}

/**
 * Sets the rotation snapping level to the specified value.
 *
 * This method updates the rotation snapping level used in the application.
 * The snapping level determines the granularity of rotation operations.
 *
 * @param NewRotationSnap The new rotation snapping level to set.
 *                        It must be a valid value of the ERotationSnap enumeration.
 */
void USnapSettingsViewModel::SetRotationSnap(ERotationSnap NewRotationSnap)
{
	UE_MVVM_SET_PROPERTY_VALUE(RotationSnap, NewRotationSnap);
}

EGridSnap USnapSettingsViewModel::GetGridSnap() const
{
	return GridSnap;
}

ERotationSnap USnapSettingsViewModel::GetRotationSnap() const
{
	return RotationSnap;
}

/**
 * Cycles through the available grid snapping levels in a sequential manner.
 *
 * This method increments the current grid snapping level to the next level
 * in the EGridSnap enumeration. When the last level is reached, it wraps
 * around to the first level in the enumeration.
 *
 * The new snapping level is set using the SetGridSnap method.
 */
void USnapSettingsViewModel::CycleGridSnap()
{
	const uint8 CurrentGridSnap = static_cast<uint8>(GridSnap);
	const uint8 NewGridSnap = (CurrentGridSnap + 1) % static_cast<uint8>(EGridSnap::Count);
	SetGridSnap(static_cast<EGridSnap>(NewGridSnap));
}

void USnapSettingsViewModel::CycleRotationSnap()
{
	uint8 CurrentRotSnap = static_cast<uint8>(RotationSnap);
	uint8 NewRotSnap = (CurrentRotSnap + 1) % static_cast<uint8>(ERotationSnap::Count);
	SetRotationSnap(static_cast<ERotationSnap>(NewRotSnap));
}
