// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpacetimeHttpClient.generated.h"


/**
 * Holds the identity information returned by the server.
 */
USTRUCT(BlueprintType)
struct FIdentityInfo
{
	GENERATED_BODY()

	/** The unique identity assigned by SpacetimeDB */
	UPROPERTY(BlueprintReadOnly, Category="SpacetimeDB|Auth")
	FString Id;

	/** The authentication token for future requests */
	UPROPERTY(BlueprintReadOnly, Category="SpacetimeDB|Auth")
	FString Token;
};


/**
 * 
 */
UCLASS()
class SPACETIMEDBCORE_API USpacetimeHttpClient : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Http")
	static bool CreateIdentity(const FString& ServerURI, FIdentityInfo& OutIdentity, FString& OutError);
};
