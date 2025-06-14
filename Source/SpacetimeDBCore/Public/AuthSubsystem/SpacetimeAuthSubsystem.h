// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Http/SpacetimeHttpClient.h"

#include "SpacetimeAuthSubsystem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, meta=(DisplayName="SpacetimeDB Authentication"))
class SPACETIMEDBCORE_API USpacetimeAuthSubsystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:	
	/** Loads the token file (if present); returns true if one was loaded. */
	UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Auth")
	bool LoadIdentity();

	/** Persists the supplied token string to disk. */
	UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Auth")
	bool SaveIdentity(const FIdentityInfo& Identity);

	/** Returns the in-memory token (empty if none loaded). */
	UFUNCTION(BlueprintPure, Category="SpacetimeDB|Auth")
	FIdentityInfo GetIdentity() const { return CachedIdentity; }

	/** Deletes the file + clears cache, forcing a fresh login next time. */
	UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Auth")
	void ClearIdentity();

	/** Deletes the file + clears cache, forcing a fresh login next time. */
	UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Auth")
	bool CreateIdentity(const FString& ServerURI, FString& OutError);

private:
	static FString GetTokenFilePath();
	static FString GetUserDataBaseDir();

	/** Holds the token for this session. */
	FIdentityInfo CachedIdentity;
};
