// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/IoroomPlayerState.h"

#include "MVVMGameSubsystem.h"
#include "TimerManager.h"
#include "Core/IoroomGameState.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/DesktopPawn.h"
#include "UI/ViewModels/SnapSettingsViewModel.h"

static const FName SnapSettingsContextName(TEXT("SnapSettings"));


void AIoroomPlayerState::BeginPlay()
{
	Super::BeginPlay();

	InitializePlayerColor();
	InitializeViewModels();
}

/*
 * Server-only: reserves a stencil slot from the GameState pool and derives a unique hue from it.
 * Slot-based hue (step of 25/255) guarantees visually distinct colors across all 10 players
 * without risking duplicates that a random approach would produce.
 */
void AIoroomPlayerState::InitializePlayerColor()
{
	if (!HasAuthority()) return;
	
	UWorld* World = GetWorld();
	if (!World) return;

	AIoroomGameState* IoroomGameState = World->GetGameState<AIoroomGameState>();
	if (!IoroomGameState) return;
	
	StencilSlot = IoroomGameState->ReserveStencilSlot();

	
	AssignedColor = 
		UKismetMaterialLibrary::GetVectorParameterValue(
			GetWorld(), PlayerColorMPC, FName(FString::Printf(TEXT("PlayerColor_%d"), StencilSlot - 1))
		);
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this] ()
	{
		if (ADesktopPawn* Pawn = GetPawn<ADesktopPawn>()) Pawn->ApplyPlayerVisuals( AssignedColor);
	});
}

/**
 * @brief Initializes the view models required by the player state.
 *
 * This method sets up the `SnapSettingsViewModel` for managing grid and rotation snapping settings.
 * It ensures that the player controller is valid, the controller is local, and the game instance exists
 * before proceeding with initialization. The `SnapSettingsViewModel` is created and populated with
 * default snapping values for the grid and rotation.
 *
 * If the `UMVVMGameSubsystem` and its associated `UMVVMViewModelCollectionObject` are available in the
 * game instance, the `SnapSettingsViewModel` is added to the collection with the provided context name.
 *
 * @note This method should only be called for local controllers.
 * @note The required context name for `SnapSettingsViewModel` is stored in a static variable
 *       `SnapSettingsContextName`.
 *
 * @warning This method exits early if any of the required components (like `PlayerController`,
 *          `GameInstance`, `UMVVMGameSubsystem`, or `ViewModelCollection`) are unavailable.
 */
void AIoroomPlayerState::InitializeViewModels()
{
	UE_LOG(LogTemp, Log, TEXT("InitializeViewModels: %s, NetMode=%d"), *GetPlayerName(), (int32)GetNetMode());
	
	const APlayerController* PlayerController = GetPlayerController();
	if (!PlayerController) {UE_LOG(LogTemp, Warning, TEXT("exit: No Player Controller")); return;}
	if (!PlayerController->IsLocalController()) {UE_LOG(LogTemp, Warning, TEXT("exit: No Local Player Controller")); return;}
	if (!GetGameInstance()) {UE_LOG(LogTemp, Warning, TEXT("exit: No Gameinstance")); return;}
	
	SnapSettingsViewModel = NewObject<USnapSettingsViewModel>(this);
	SnapSettingsViewModel->SetGridSnap(GridSnap);
	SnapSettingsViewModel->SetRotationSnap(RotationSnap);

	const UMVVMGameSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMVVMGameSubsystem>();
	if (!Subsystem) {UE_LOG(LogTemp, Warning, TEXT("exit: No UMVVMGameSubsystem found")); return;}
	
	UMVVMViewModelCollectionObject* ViewModelCollection = Subsystem->GetViewModelCollection();
	if (!ViewModelCollection) return;
	FMVVMViewModelContext Context(USnapSettingsViewModel::StaticClass(), SnapSettingsContextName);
	if (ensure(ViewModelCollection->AddViewModelInstance(Context, SnapSettingsViewModel)))
	{
		UE_LOG(LogTemp, Log, TEXT("MVVMSnapSetting registered"));
		OnViewModelsReady();
	}
}

void AIoroomPlayerState::RemoveViewModels()
{
	if (!SnapSettingsViewModel) return;
	if (!GetGameInstance()) return;
	
	UMVVMGameSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMVVMGameSubsystem>();
	if (!Subsystem) return;
	
	UMVVMViewModelCollectionObject* ViewModelCollection = Subsystem->GetViewModelCollection();
	if (!ViewModelCollection) return;
	FMVVMViewModelContext Context(USnapSettingsViewModel::StaticClass(), SnapSettingsContextName);
	ViewModelCollection->RemoveViewModel(Context);
}

/*
 * Releases the stencil slot back to the pool when the player leaves,
 * making it available for the next incoming player.
 */
void AIoroomPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (HasAuthority())
	{
		AIoroomGameState* IoroomGameState = GetWorld()->GetGameState<AIoroomGameState>();
		if (IoroomGameState) IoroomGameState->ReleaseStencilSlot(StencilSlot);
	}
	
	RemoveViewModels();
}

/*
 * Both properties use ReplicatedUsing to cover all replication orderings.
 * OnRep_StencilSlot handles the case where StencilSlot arrives after AssignedColor.
 * OnRep_AssignedColor handles the case where AssignedColor arrives after StencilSlot.
 * ADesktopPawn::OnRep_PlayerState handles the case where the pawn link arrives last.
 */
void AIoroomPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIoroomPlayerState, AssignedColor)
	DOREPLIFETIME(AIoroomPlayerState, StencilSlot)
}

void AIoroomPlayerState::OnRep_StencilSlot()
{
	ADesktopPawn* Pawn = GetPawn<ADesktopPawn>();
	if (Pawn) Pawn->ApplyPlayerVisuals(AssignedColor);
}

/*
 * Fires on clients when AssignedColor replicates. Covers the case where AssignedColor arrives
 * after PawnPrivate is already set. The reverse case (PawnPrivate arrives after AssignedColor)
 * is handled by ADesktopPawn::OnRep_PlayerState.
 */
void AIoroomPlayerState::OnRep_AssignedColor()
{
	ADesktopPawn* Pawn = GetPawn<ADesktopPawn>();
	if (Pawn) Pawn->ApplyPlayerVisuals(AssignedColor);
}

// return float value based on enum GridSnap
float AIoroomPlayerState::GetGridStep() const
{
	switch (GridSnap)
	{
		case EGridSnap::Minimum: return 1.f;
		case EGridSnap::Fine: return 5.f;
		case EGridSnap::Small: return 20.f;
		case EGridSnap::Medium: return 50.f;
		case EGridSnap::Large: return 100.f;
		default: return 20.f;
	}
}

// return float value based on enum RotationSnap
float AIoroomPlayerState::GetRotStep() const
{
	switch (RotationSnap)
	{
		case ERotationSnap::Minimum: return 1.f;
		case ERotationSnap::Fine: return 5.f;
		case ERotationSnap::Small: return 15.f;
		case ERotationSnap::Medium: return 45.f;
		case ERotationSnap::Large: return 90.f;
		default: return 15.f;
	}
}
