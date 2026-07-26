// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Types/IoroomTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IoroomUIFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UE_IOROOMPROJECT_API UIoroomUIFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, meta=(DisplayName="Grid Snap To Text"))
	static FText GridSnapToText(EGridSnap Snap);
	UFUNCTION(BlueprintPure, meta=(DisplayName="Rotation Snap To Text"))
	static FText RotationSnapToText(ERotationSnap Snap);
};
