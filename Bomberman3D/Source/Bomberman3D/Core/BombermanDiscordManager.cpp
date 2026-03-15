// Copyright (c) 2026, Michal Flaška & RedFox Studios. All Rights Reserved.

#include "Core/BombermanDiscordManager.h"

#define DISCORD_APP_ID 1482825420733808791 // appid nigga

void UBombermanDiscordManager::Init(ULocalPlayer* LocalPlayer)
{
	Discord = ULocalPlayer::GetSubsystem<UDiscordLocalPlayerSubsystem>(LocalPlayer);
	if (!Discord) return;

	auto LogCallback = FDiscordClientLogCallback::CreateUObject(this, &UBombermanDiscordManager::OnLogMessage);
	FScriptDelegate StatusChanged;
	Discord->Client->AddLogCallback(LogCallback, EDiscordLoggingSeverity::Info);
	Discord->OnStatusChanged.AddDynamic(this, &UBombermanDiscordManager::OnStatusChanged);

	Connect();
}

void UBombermanDiscordManager::Connect()
{
	if (!Discord) return;

	CodeVerifier = Discord->Client->CreateAuthorizationCodeVerifier();
	auto AuthArgs = NewObject<UDiscordAuthorizationArgs>();
	AuthArgs->Init();
	AuthArgs->SetClientId(DISCORD_APP_ID);
	AuthArgs->SetScopes(UDiscordClient::GetDefaultPresenceScopes());
	AuthArgs->SetCodeChallenge(CodeVerifier->Challenge());
	Discord->Client->Authorize(AuthArgs, FDiscordClientAuthorizationCallback::CreateUObject(this, &UBombermanDiscordManager::OnAuthorizeCompleted));
}

void UBombermanDiscordManager::UpdatePresence(int32 Stage, int32 Lives, bool bInMainMenu)
{
	if (!Discord || !bConnected) return;

	UDiscordActivity* Activity = NewObject<UDiscordActivity>();
	Activity->Init();
	Activity->SetType(EDiscordActivityTypes::Playing);

	if (bInMainMenu)
	{
		Activity->SetState("In Main Menu");
		Activity->SetDetails("Bomberman3D");
	}
	else
	{
		Activity->SetDetails(FString::Printf(TEXT("Stage %d / 50"), Stage));
		Activity->SetState(FString::Printf(TEXT("%d lives remaining - Singleplayer"), Lives));
	}

	Discord->Client->UpdateRichPresence(Activity, FDiscordClientUpdateRichPresenceCallback::CreateUObject(this, &UBombermanDiscordManager::OnRichPresenceUpdated));
}

void UBombermanDiscordManager::OnLogMessage(FString Message, EDiscordLoggingSeverity Severity)
{
	UE_LOG(LogTemp, Log, TEXT("[Discord] [%s] %s"), *UEnum::GetValueAsString(Severity), *Message);
}

void UBombermanDiscordManager::OnStatusChanged(EDiscordClientStatus Status, EDiscordClientError Error, int32 ErrorDetail)
{
	UE_LOG(LogTemp, Warning, TEXT("[Discord] Status changed: %s"), *UEnum::GetValueAsString(Status));

	if (Status == EDiscordClientStatus::Ready)
	{
		bConnected = true;
		UE_LOG(LogTemp, Warning, TEXT("[Discord] Connected!"));
		UpdatePresence(1, 3, true);
	}
	else if (Error != EDiscordClientError::None)
	{
		UE_LOG(LogTemp, Error, TEXT("[Discord] Error: %s (Detail: %d)"), *UEnum::GetValueAsString(Error), ErrorDetail);
	}
}

void UBombermanDiscordManager::OnAuthorizeCompleted(UDiscordClientResult* Result, FString Code, FString RedirectUri)
{
	if (!Result->Successful())
	{
		UE_LOG(LogTemp, Error, TEXT("[Discord] Auth failed: %s"), *Result->Error());
		return;
	}

	Discord->Client->GetToken(DISCORD_APP_ID, Code, CodeVerifier->Verifier(), RedirectUri, FDiscordClientTokenExchangeCallback::CreateUObject(this, &UBombermanDiscordManager::OnTokenExchange));
}

void UBombermanDiscordManager::OnTokenExchange(UDiscordClientResult* Result, FString AccessToken, FString RefreshToken, EDiscordAuthorizationTokenType TokenType, int32 ExpiresIn, FString Scope)
{
	if (!Result->Successful())
	{
		UE_LOG(LogTemp, Error, TEXT("[Discord] Token exchange failed: %s"), *Result->Error());
		return;
	}

	Discord->Client->UpdateToken(TokenType, AccessToken, FDiscordClientUpdateTokenCallback::CreateUObject(this, &UBombermanDiscordManager::OnTokenUpdated));
}

void UBombermanDiscordManager::OnTokenUpdated(UDiscordClientResult* Result)
{
	Discord->Client->Connect();
}

void UBombermanDiscordManager::OnRichPresenceUpdated(UDiscordClientResult* Result)
{
	if (!Result->Successful())
	{
		UE_LOG(LogTemp, Error, TEXT("[Discord] Rich Presence update failed"));
	}
}
