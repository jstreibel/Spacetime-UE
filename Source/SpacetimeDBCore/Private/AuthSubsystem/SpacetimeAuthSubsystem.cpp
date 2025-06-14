// Fill out your copyright notice in the Description page of Project Settings.


#include "AuthSubsystem/SpacetimeAuthSubsystem.h"

#include "Http/SpacetimeHttpClient.h"

#include "toml.hpp"

bool USpacetimeAuthSubsystem::LoadIdentity()
{
	const FString Path = GetTokenFilePath();
	if (FString Content; FFileHelper::LoadFileToString(Content, *Path))
	{
		const FTCHARToUTF8 Utf8Conv(*Content);
		const size_t RawLength = Utf8Conv.Length();
		const auto StdContent = std::string(Utf8Conv.Get(), RawLength);
		
		try
		{
			// This returns a toml::table on success, or throws toml::parse_error on failure:
			toml::table Table = toml::parse(StdContent);

			const auto Id = **Table.get_as<std::string>("id");
			const auto Token = **Table.get_as<std::string>("token");
			CachedIdentity.Id = StringCast<TCHAR>(Id.c_str());
			CachedIdentity.Token = StringCast<TCHAR>(Token.c_str());

			UE_LOG(LogTemp, Log, TEXT("Parsed TOML → User id=\"%s\""), *CachedIdentity.Id);
		}
		catch (const toml::parse_error& Err)
		{
			const FString Msg = FString::Printf(TEXT("TOML parse error: %s (at %u:%u)"),
										  UTF8_TO_TCHAR(Err.what()),
										  Err.source().begin.line,
										  Err.source().begin.column);
			
			UE_LOG(LogTemp, Error, TEXT("%s"), *Msg);

			return false;
		}
		
		return true;
	}
	
	return false;

}

bool USpacetimeAuthSubsystem::SaveIdentity(const FIdentityInfo& Identity)
{
	const FString Path = GetTokenFilePath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);

	std::string Id = TCHAR_TO_UTF8(*Identity.Id);
	std::string Token = TCHAR_TO_UTF8(*Identity.Token);
	
	toml::table Table;	
	Table.insert("id", Id);
	Table.insert("token", Token);
	
	std::ostringstream Out;
	Out << Table;
	const FString TCharContent(UTF8_TO_TCHAR(Out.str().c_str()));
	
	if (FFileHelper::SaveStringToFile(TCharContent, *Path))
	{
		UE_LOG(LogTemp, Log, TEXT("Saved user Identy to TOML → %s"), *Path);
		return true;
	}
	return false;
	
}

void USpacetimeAuthSubsystem::ClearIdentity()
{
	CachedIdentity.Id.Empty();
	CachedIdentity.Token.Empty();
	
	IFileManager::Get().Delete(*GetTokenFilePath());
}

bool USpacetimeAuthSubsystem::CreateIdentity(const FString& ServerURI, FString& OutError)
{
	if (!USpacetimeHttpClient::CreateIdentity(ServerURI, CachedIdentity, OutError))
	{
		OutError = FString::Printf(TEXT("Failed to create identity: %s"), *OutError);
		return false;
	}

	return true;
}

FString USpacetimeAuthSubsystem::GetTokenFilePath()
{
	FString Base = GetUserDataBaseDir();
	FString Dir  = FPaths::Combine(Base, TEXT("SpacetimeDB"));
	IFileManager::Get().MakeDirectory(*Dir, /* Tree = */ true);
	return FPaths::Combine(Dir, TEXT("AuthToken.toml"));
}

FString USpacetimeAuthSubsystem::GetUserDataBaseDir()
{
	if (FApp::IsGame())
	{
		// In a standalone or packaged build → use the OS’s per-user settings folder
		return FPlatformProcess::UserSettingsDir();
	}
	
	// In the Editor (or PIE) → use Project/Saved/
	return FPaths::ProjectSavedDir();
}
