#include "CLI/SpacetimeCLIHelper.h"

#include "CLI/toml.hpp"

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

bool FSpacetimeCLIHelper::GetCliConfig(FSpacetimeCliConfig& OutConfig, FString& OutError)
{
	const FString Path = GetDefaultCliTomlPath();
	if (!ValidateFile(Path, OutError))
	{
		return false;
	}
	return ParseToml(Path, OutConfig, OutError);
}

bool FSpacetimeCLIHelper::ValidateFile(const FString& InPath, FString& OutError)
{
	if (InPath.IsEmpty())
	{
		OutError = TEXT("Unable to determine cli.toml path.");
		return false;
	}
	IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
	if (!PF.FileExists(*InPath))
	{
		OutError = FString::Printf(TEXT("Configuration file not found: %s"), *InPath);
		return false;
	}
	if (PF.FileSize(*InPath) <= 0)
	{
		OutError = FString::Printf(TEXT("Configuration file is empty: %s"), *InPath);
		return false;
	}
	return true;
}

bool FSpacetimeCLIHelper::ParseToml(const FString& InPath, FSpacetimeCliConfig& OutConfig, FString& OutError)
{
	toml::table Table;
    try
    {
        Table = toml::parse_file(TCHAR_TO_UTF8(*InPath));
    }
    catch (const toml::parse_error& Ex)
    {
        OutError = FString::Printf(TEXT("TOML parse error: %s"), ANSI_TO_TCHAR(Ex.what()));
        return false;
    }

    auto ReadString = [&](const char* Key, FString& Dest) -> bool
    {
        if (auto Node = Table[Key].as_string())
        {
            Dest = UTF8_TO_TCHAR(Node->get().c_str());
            return true;
        }
        OutError = FString::Printf(TEXT("Missing or invalid '%s' in cli.toml"), UTF8_TO_TCHAR(Key));
        return false;
    };

    if (!ReadString("default_server", OutConfig.DefaultServer) ||
        !ReadString("web_session_token", OutConfig.WebSessionToken) ||
        !ReadString("spacetimedb_token", OutConfig.SpacetimeDBToken))
    {
        return false;
    }

    if (auto Array = Table["server_configs"].as_array())
    {
        for (auto& Item : *Array)
        {
            if (auto Sub = Item.as_table())
            {
                FSpacetimeServerConfig Cfg;
                if (auto Nick = Sub->get("nickname")->value<std::string>(); Sub->get("nickname")->is_string())
                {
                    Cfg.Nickname = UTF8_TO_TCHAR(Nick->c_str());
                }
                else { OutError = TEXT("Invalid 'nickname' in server_configs"); return false; }

                if (auto Host = Sub->get("host")->value<std::string>(); Sub->get("host")->is_string())
                {
                    Cfg.Host = UTF8_TO_TCHAR(Host->c_str());
                }
                else { OutError = TEXT("Invalid 'host' in server_configs"); return false; }

                if (auto Prot = Sub->get("protocol")->value<std::string>(); Sub->get("protocol")->is_string())
                {
                    Cfg.Protocol = UTF8_TO_TCHAR(Prot->c_str());
                }
                else { OutError = TEXT("Invalid 'protocol' in server_configs"); return false; }

                OutConfig.ServerConfigs.Add(MoveTemp(Cfg));
            }
            else
            {
                OutError = TEXT("Expected table entry in 'server_configs'");
                return false;
            }
        }
    }
    else
    {
        OutError = TEXT("Missing or invalid 'server_configs' array");
        return false;
    }

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
