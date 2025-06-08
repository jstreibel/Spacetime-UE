#include "CLI/SpacetimeCLIHelper.h"

bool FSpacetimeCLIHelper::IsCliAvailable()
{
	int32 ReturnCode;
	FString OutStdOut, OutStdErr;
	
	return FPlatformProcess::ExecProcess(
		TEXT("spacetime"),
		TEXT(""), &ReturnCode, &OutStdOut, &OutStdErr);
}

bool FSpacetimeCLIHelper::IsLoggedIn()
{
	int32 ReturnCode;
	FString OutStdOut, OutStdErr;
	
	if (!FPlatformProcess::ExecProcess(
		TEXT("spacetime"),
		TEXT("login show"), &ReturnCode, &OutStdOut, &OutStdErr))
	{
		return false;
	}

	if (ReturnCode != 0) return false;

	return OutStdOut.Contains("You are logged in as ");
}

bool FSpacetimeCLIHelper::TryParseSpacetimeLogin(const FString& CliOutput, FSpacetimeCredentials& OutCredentials)
{
	OutCredentials = FSpacetimeCredentials();

	// Break into lines
	TArray<FString> Lines;
	CliOutput.ParseIntoArrayLines(Lines, /*InCullEmpty*/ true);

	const FString IdentityPrefix = TEXT("You are logged in as ");
	const FString TokenPrefix    = TEXT("Your auth token (don't share this!) is ");

	for (const FString& RawLine : Lines)
	{
		FString Line = RawLine.TrimStartAndEnd();

		if (Line.StartsWith(IdentityPrefix))
		{
			// Everything after the prefix is the identity
			OutCredentials.Identity = Line.Mid(IdentityPrefix.Len());
		}
		else if (Line.StartsWith(TokenPrefix))
		{
			// Everything after the prefix is the JWT
			OutCredentials.Token = Line.Mid(TokenPrefix.Len());
		}
	}

	return OutCredentials.IsValid();
}

bool FSpacetimeCLIHelper::GetCliTomlPath(FString& OutPath, FString& OutError)
{
	// Determine default location
	FString Candidate = GetDefaultCliTomlPath();
	if (Candidate.IsEmpty())
	{
		OutError = TEXT("Unable to determine the default SpacetimeDB CLI config path.");
		return false;
	}

	// Check file existence
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*Candidate))
	{
		OutError = FString::Printf(TEXT("Configuration file not found: %s"), *Candidate);
		return false;
	}

	// Check file size (non-empty)
	int64 FileSize = PlatformFile.FileSize(*Candidate);
	if (FileSize <= 0)
	{
		OutError = FString::Printf(TEXT("Configuration file is empty: %s"), *Candidate);
		return false;
	}

	OutPath = Candidate;
	return true;
}

FString FSpacetimeCLIHelper::GetDefaultCliTomlPath()
{
#if PLATFORM_WINDOWS
	// Windows: %LOCALAPPDATA%\SpacetimeDB\cli.toml
	FString LocalAppData = FPlatformMisc::GetEnvironmentVariable(TEXT("LOCALAPPDATA"));
	if (!LocalAppData.IsEmpty())
	{
		return FPaths::Combine(LocalAppData, TEXT("SpacetimeDB"), TEXT("cli.toml"));
	}
	// Fallback: %USERPROFILE%\AppData\Local\SpacetimeDB\cli.toml
	FString UserProfile = FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"));
	if (!UserProfile.IsEmpty())
	{
		return FPaths::Combine(UserProfile, TEXT("AppData"), TEXT("Local"), TEXT("SpacetimeDB"), TEXT("cli.toml"));
	}
	return TEXT("");

#elif PLATFORM_MAC || PLATFORM_LINUX
	// macOS/Linux: $XDG_CONFIG_HOME/spacetime/cli.toml or ~/.config/spacetime/cli.toml
	FString ConfigHome = FPlatformMisc::GetEnvironmentVariable(TEXT("XDG_CONFIG_HOME"));
	if (ConfigHome.IsEmpty())
	{
		FString Home = FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"));
		if (!Home.IsEmpty())
		{
			ConfigHome = FPaths::Combine(Home, TEXT(".config"));
		}
	}
	if (!ConfigHome.IsEmpty())
	{
		return FPaths::Combine(ConfigHome, TEXT("spacetime"), TEXT("cli.toml"));
	}
	return TEXT("");

#else
	return TEXT("");
#endif
}
