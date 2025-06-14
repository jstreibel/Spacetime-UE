// SpacetimeCLIHelper.h
#pragma once

#include "CoreMinimal.h"

/**
 * Struct representing a single [[server_configs]] entry in cli.toml.
 */
struct FSpacetimeServerConfig
{

	/** Human-readable nickname ("local", "maincloud", etc.) */
	FString Nickname;

	/** Host address and port (e.g. "127.0.0.1:3000") */
	FString Host;

	/** Protocol ("http" or "https") */
	FString Protocol;

	FString GetURI() const { return FString::Printf(TEXT("%ls://%ls"), *Protocol, *Host); }
};

/**
 * Struct representing the parsed contents of cli.toml.
 */
struct FSpacetimeCliConfig
{
	/** default_server = "..." */
	FString DefaultServer;

	/** web_session_token = "..." */
	FString WebSessionToken;

	/** spacetimedb_token = "..." */
	FString SpacetimeDBToken;

	/** [[server_configs]] array */
	TArray<FSpacetimeServerConfig> ServerConfigs;
};