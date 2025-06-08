#pragma once

#include "CoreMinimal.h"

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

private:
	/** 
	 * @return Absolute path to the default ~/.config/spacetime/cli.toml  (Linux/macOS)
	 *         or %LOCALAPPDATA%\SpacetimeDB\cli.toml (Windows), 
	 *         or empty if neither could be determined.
	 */
	static FString GetDefaultCliTomlPath();
};
