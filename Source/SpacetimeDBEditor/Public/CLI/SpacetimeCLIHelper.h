#pragma once

#include "CoreMinimal.h"
#include "SpacetimeCliConfig.h"

/**
 * Helper for locating the per-user SpacetimeDB CLI config file (cli.toml).
 */
class FSpacetimeCLIHelper
{
public:

	static bool IsCliAvailable();

	static bool IsLoggedIn();

	struct FSpacetimeCredentials
	{
		FString Identity;
		FString Token;

		bool IsValid() const
		{
			return !Identity.IsEmpty() && !Token.IsEmpty();
		}
	};
	
	static bool TryParseSpacetimeLogin(const FString& CliOutput, FSpacetimeCredentials& OutCredentials);

	/**
	 * Returns the default path for cli.toml (calls GetDefaultCliTomlPath internally).
	 * If the file exists and is non-empty, returns true and sets OutPath.
	 * Otherwise returns false and sets OutError.
	 */
	static bool GetCliTomlPath(FString& OutPath, FString& OutError);

	/**
	 * Finds and parses the user's cli.toml into OutConfig.
	 * @param OutConfig Parsed contents on success.
	 * @param OutError  Error message on failure.
	 * @return true if the file was found, valid TOML, and correctly parsed.
	 */
	static bool GetCliConfig(FSpacetimeCliConfig& OutConfig, FString& OutError);

private:
	/** Validates existence and non-emptiness of a given path. */
	static bool ValidateFile(const FString& InPath, FString& OutError);

	/** Reads and parses the TOML at InPath, then fills OutConfig. */
	static bool ParseToml(const FString& InPath, FSpacetimeCliConfig& OutConfig, FString& OutError);
	
	/** 
	 * @return Absolute path to the default ~/.config/spacetime/cli.toml  (Linux/macOS)
	 *         or %LOCALAPPDATA%\SpacetimeDB\cli.toml (Windows), 
	 *         or empty if neither could be determined.
	 */
	static FString GetDefaultCliTomlPath();
};
