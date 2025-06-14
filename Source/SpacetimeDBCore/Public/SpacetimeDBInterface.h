#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SpacetimeDBInterface.generated.h"

UCLASS(Blueprintable)
class SPACETIMEDBCORE_API USpacetimeDBInterface : public UObject
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpacetimeDB")
	FString SpacetimeDBModuleName;

	/** 
	 *  Returns the stored token for ServerURI, or if none exists,
	 *  POSTs to /v1/identity, saves the returned token, and returns it.
	 */
	UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Auth")
	FString GetUserToken(const FString& ServerURI);
	
};
