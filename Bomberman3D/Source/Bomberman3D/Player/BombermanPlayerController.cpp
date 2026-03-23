// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Player/BombermanPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void ABombermanPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (HUDWidgetClass)
	{
		UUserWidget* HUD = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUD) HUD->AddToViewport();
	}

	if (PauseAction)
	{
		PauseAction->bTriggerWhenPaused = true;
	}
}

void ABombermanPlayerController::UpdateAudioListener()
{
	if (APawn* P = GetPawn())
	{
		SetAudioListenerOverride(nullptr, P->GetActorLocation(), FRotator::ZeroRotator);
	}
}
