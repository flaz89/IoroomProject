// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IoroomGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UE_IOROOMPROJECT_API UIoroomGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	// function called before any BeginPlay() perfect for set or detect things
	virtual void Init() override; 
	
	// function called automatically at first launch
	UFUNCTION(BlueprintCallable, Category = "Graphics")
	void AutodetectGraphicsSettings();
	
	// function called to SET level of rendering scalability
	UFUNCTION(BlueprintCallable, Category = "Graphics")
	void SetScalabilityLevel(int32 Level);
	
	// function called to GET level of rendering scalability
	UFUNCTION(BlueprintCallable, Category = "Graphics")
	int32 GetCurrentScalabilityLevel() const;
	
	// function called to GET the name of GPU detected
	UFUNCTION(BlueprintCallable, Category = "Graphics")
	FString GetDetectedGPUName() const {return CachedGPUName;};
	
	// function called to GET the VRAM in Mb of GPU detected
	UFUNCTION(BlueprintCallable, Category = "Graphics")
	int32 GetDetectedVRAM() const {return DetectedVRAMMB;};
	
protected:
	// spawn scene test, test FPS, return best tier solution
	int32 RunGraphicsBenchmark();
	
	// detect GPU name through RHI (Rendering Hardware Interface) 
	FString DetectGPUName();
	
	// get he quantity in MB of GPU memory
	int32 GetGPUMemoryMB() const;
	
	// parse GPU name, check VRAM and set tier
	int32 DetermineTierScalabilityLevel();
	
private:
	
	int32 DetectedVRAMMB{};
	FString CachedGPUName{};
};
