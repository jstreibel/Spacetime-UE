// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HttpFwd.h"
#include "Kismet/BlueprintAsyncActionBase.h"
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
	FString AuthToken;
};


USTRUCT(BlueprintType)
struct FConnectionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="SpacetimeDB|Connection")
	FIdentityInfo IdentityIndo;

	UPROPERTY(BlueprintReadOnly, Category="SpacetimeDB|Connection")
	FString ServerURI;
	FString ServerName;

	// Some other info we might want to expose:
	// FString ServerVersion;
	// FString ServerDescription;
	// FString ServerLicense;
	// FString ServerLicenseURL;
	// FString ServerLicenseType;
	// FString ServerLicenseVersion;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIdentitySuccess, FIdentityInfo, Identity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIdentityError  , FString     , ErrorMessage);

/**
 * 
 */
UCLASS()
class SPACETIMEDBCORE_API USpacetimeHttpClient : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Blueprint nodes will let users hook these up
	UPROPERTY(BlueprintAssignable)
	FOnIdentitySuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnIdentityError OnError;
	
	UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Http")
	static USpacetimeHttpClient* CreateIdentity(const FString& ServerURI);

	// UObject interface: this is called automatically after the node is created
	virtual void Activate() override;

private:
	// Bound to the FHttpRequest’s completion
	void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// Stored so we can kick off the request in Activate()
	FString URL;

};
