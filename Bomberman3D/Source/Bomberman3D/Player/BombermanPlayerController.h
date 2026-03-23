// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "Blueprint/UserWidget.h"

#include "BombermanPlayerController.generated.h"

/**
 * 
 */
UCLASS()

class BOMBERMAN3D_API ABombermanPlayerController : public APlayerController
{
	GENERATED_BODY()

  protected:
	virtual void BeginPlay() override;
	virtual void UpdateAudioListener();

  public:
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	// known UE issue -> Enhanced Input doesnt fire when the game is paused by default. so here am i, fixing it in C++
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* PauseAction;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> HUDWidgetClass;
};
