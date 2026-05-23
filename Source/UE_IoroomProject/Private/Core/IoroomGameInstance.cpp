// Fill out your copyright notice in the Description page of Project Settings.


#include "UE_IoroomProject/Public/Core/IoroomGameInstance.h"
#include "RHIGlobals.h"
#include "RHIStats.h"

void UIoroomGameInstance::Init()
{
	Super::Init();
	
	// detect hardware
	CachedGPUName = DetectGPUName();
	DetectedVRAMMB = GetGPUMemoryMB();
	
}

void UIoroomGameInstance::AutodetectGraphicsSettings()
{
}

void UIoroomGameInstance::SetScalabilityLevel(int32 Level)
{
}

int32 UIoroomGameInstance::RunGraphicsBenchmark()
{
	return 0;
}

FString UIoroomGameInstance::DetectGPUName()
{
	FString GPUName = GRHIAdapterName; // global RHIGlobals.h
	if (GPUName.IsEmpty()) GPUName = TEXT("GPU name unknown");
	UE_LOG(LogTemp, Warning, TEXT("GPU Name: %s"), *GPUName)
	return GPUName;
}

int32 UIoroomGameInstance::GetGPUMemoryMB() const
{
	FTextureMemoryStats TextureStats; // global RHIStats.h
	RHIGetTextureMemoryStats(TextureStats);
	int32 Memory = TextureStats.TotalGraphicsMemory / (1024 * 1024);
	UE_LOG(LogTemp, Warning, TEXT("GPU Memory: %d"), Memory)
	return TextureStats.TotalGraphicsMemory / (1024 * 1024);
}

int32 UIoroomGameInstance::DetermineTierScalabilityLevel()
{
	return 0;
}
