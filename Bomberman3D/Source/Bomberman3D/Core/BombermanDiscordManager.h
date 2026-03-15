// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
// #include "UObject/NoExportTypes.h"
#include "DiscordLocalPlayerSubsystem.h"
#include "BombermanDiscordManager.generated.h"

UCLASS()

class BOMBERMAN3D_API UBombermanDiscordManager : public UObject
{
	GENERATED_BODY()

  public:
	void Init(ULocalPlayer* LocalPlayer);
	void UpdatePresence(int32 Stage, int32 Lives, bool bInMainMenu);

  private:
	UPROPERTY()
	UDiscordLocalPlayerSubsystem* Discord = nullptr;

	UPROPERTY()
	UDiscordAuthorizationCodeVerifier* CodeVerifier = nullptr;

	bool bConnected = false;

	void Connect();
	void OnLogMessage(FString Message, EDiscordLoggingSeverity Severity);

	UFUNCTION()
	void OnStatusChanged(EDiscordClientStatus Status, EDiscordClientError Error, int32 ErrorDetail);

	void OnAuthorizeCompleted(UDiscordClientResult* Result, FString Code, FString RedirectUri);
	void OnTokenExchange(UDiscordClientResult* Result, FString AccessToken, FString RefreshToken, EDiscordAuthorizationTokenType TokenType, int32 ExpiresIn, FString Scope);
	void OnTokenUpdated(UDiscordClientResult* Result);
	void OnRichPresenceUpdated(UDiscordClientResult* Result);
};
