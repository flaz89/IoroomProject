// Fill out your copyright notice in the Description page of Project Settings.


#include "UE_IoroomProject/Public/Core/IoroomGameInstance.h"
#include "RHIGlobals.h"
#include "RHIStats.h"
#include "Scalability.h"
#include "GameFramework/GameUserSettings.h"

/*
 * STEPS:
 * - fisrt detect hardware and print the output
 * - second check if it's first launch setting the bool bHasAutoDetected
 *				- if true auto set scalability tier level, set the GameUserSettings.ini with new bool value, write it on SSD
 *				- if false, scalability tier level already set, do nothing
 */
void UIoroomGameInstance::Init()
{
	Super::Init();
	
	CachedGPUName = DetectGPUName();
	DetectedVRAMMB = GetGPUMemoryMB();
	UE_LOG(LogTemp, Warning, TEXT("IoroomGameInstance / Init() -> Detected GPU: %s (%d MB)"), *CachedGPUName, DetectedVRAMMB);
	UE_LOG(LogTemp, Warning, TEXT("IoroomGameInstance / Init() -> Determined Tier: %d"), DetermineTierScalabilityLevel());
	
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		bool bHasAutoDetected = false;
		GConfig->GetBool(TEXT("/Script/Engine.GameUserSettings"), TEXT("bHasAutoDetectedGraphics"), bHasAutoDetected, GGameUserSettingsIni);
		
		if (!bHasAutoDetected)
		{
			UE_LOG(LogTemp, Warning, TEXT("IoroomGameInstance / Init() -> Cold start, no Graphics Settings"));
			AutodetectGraphicsSettings();
			GConfig->SetBool(TEXT("/Script/Engine.GameUserSettings"), TEXT("bHasAutoDetectedGraphics"), true, GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("IoroomGameInstance / Init() -> Scalability level already set with tier: %d"), GetCurrentScalabilityLevel());
		}
	}
}

void UIoroomGameInstance::AutodetectGraphicsSettings()
{
	int32 TierQuality = DetermineTierScalabilityLevel();
	UE_LOG(LogTemp, Warning, TEXT("IoroomGameInstance / AutodetectGraphicsSettings() -> Hardware-based tier level: %d"), TierQuality);
	
	if (TierQuality == 1 || TierQuality == 2)
	{
		const int32 BenchmarkTierQuality = RunGraphicsBenchmark();
		TierQuality = FMath::Min(TierQuality, BenchmarkTierQuality);
		UE_LOG(LogTemp, Warning, TEXT("IoroomGameInstance / AutodetectGraphicsSettings() -> Benchmark tier level: %d"), TierQuality);
	}
	
	TierQuality = FMath::Clamp(TierQuality, 0, 4);
	SetScalabilityLevel(TierQuality);
	static const FString LevelNames[] = {TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic"), TEXT("Cinematic")}; // (warning: out of bound)
	UE_LOG(LogTemp, Warning, TEXT("IoroomGameInstance / AutodetectGraphicsSettings() -> Tier Scalability level auto set is: %d - %s"), TierQuality, *LevelNames[TierQuality]);
}

void UIoroomGameInstance::SetScalabilityLevel(const int32 Level)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;
	
	Settings->SetOverallScalabilityLevel(Level);
	if (Level <= 1) Settings->SetTextureQuality(2); // force texture quality on low specs
	
	if (Level >= 3 && CachedGPUName.Contains(TEXT("RTX"), ESearchCase::IgnoreCase)) // set hardware ray tracing 
	{
		IConsoleVariable* HWRTVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.HardwareRayTracing"));
		IConsoleVariable* HWRTRefVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.Reflections.HardwareRayTracing"));
		IConsoleVariable* MeshSDFVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.TraceMeshSDFs"));
		
		if (HWRTVar) HWRTVar->Set(1, ECVF_SetByCode);
		if (HWRTRefVar) HWRTRefVar->Set(1, ECVF_SetByCode);
		if (MeshSDFVar) MeshSDFVar->Set(0, ECVF_SetByCode);
		
		UE_LOG(LogTemp, Warning, TEXT("IoroomGameInstance / SetScalabilityLevel() -> Enabled HWRT, reflections HWRT, disabled MeshSDF"));
	}
	
	Settings->ApplySettings(false); // apply & save
}

int32 UIoroomGameInstance::GetCurrentScalabilityLevel() const
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return 1; // default Medium Tier
	
	return Settings->GetOverallScalabilityLevel();
}

int32 UIoroomGameInstance::RunGraphicsBenchmark()
{
	// run built-in benchmark to select the quality tier
	const Scalability::FQualityLevels BenchmarkResults = Scalability::BenchmarkQualityLevels(); // static from "Scalability.h"
	const int32 TierQualityFromBenchmark = BenchmarkResults.ViewDistanceQuality;
	
	UE_LOG(LogTemp, Warning, TEXT("IoroomGameInstance / RunGraphicsBenchmark() -> Benchmark Tier scalability: %d"), TierQualityFromBenchmark);
	return TierQualityFromBenchmark;
}

FString UIoroomGameInstance::DetectGPUName()
{
	FString GPUName = GRHIAdapterName; // global RHIGlobals.h
	if (GPUName.IsEmpty()) GPUName = TEXT("GPU name unknown");
	
	return GPUName;
}

int32 UIoroomGameInstance::GetGPUMemoryMB() const
{
	FTextureMemoryStats TextureStats; // global RHIStats.h
	RHIGetTextureMemoryStats(TextureStats);
	
	return TextureStats.TotalGraphicsMemory / (1024 * 1024);
}

int32 UIoroomGameInstance::DetermineTierScalabilityLevel()
{
	if (CachedGPUName.Contains(TEXT("Apple")))
	{
		if (DetectedVRAMMB >= 48000) return 4; // CINEMATIC 
		if (DetectedVRAMMB >= 24000) return 3; // EPIC
		if (DetectedVRAMMB >= 10000) return 2; // HIGH
		if (DetectedVRAMMB >= 6000) return 1;  // MEDIUM
		return 0;							   // LOW
	} 

	if (DetectedVRAMMB >= 19000) return 4; // CINEMATIC 
	if (DetectedVRAMMB >= 13000) return 3; // EPIC
	if (DetectedVRAMMB >= 8000) return 2;  // HIGH
	if (DetectedVRAMMB >= 6000) return 1;  // MEDIUM
	return 0;							   // LOW
}
