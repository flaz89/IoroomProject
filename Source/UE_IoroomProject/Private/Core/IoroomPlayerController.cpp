// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/IoroomPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void AIoroomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			if (DefaultInputMappingContext != nullptr)
			{
				Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
				UE_LOG(LogTemp, Warning, TEXT("IoroomPlayerController / BeginPlay() -> IMC set is: %s"), *DefaultInputMappingContext->GetName());
			} 
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("IoroomPlayerController / BeginPlay() -> No Mapping Context referenced to Subsystem Player Controller"));
			}
		}
	}
}
