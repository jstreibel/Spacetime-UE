// Copyleft

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpacetimeBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SPACETIMEDBRUNTIME_API USpacetimeBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:
	/**
	 * Calls: spacetime describe --json <DatabaseName>
	 * Returns: true on success, and fills OutSpaces.
	 */
	UFUNCTION(BlueprintCallable,
			  Category="SpacetimeDB",
			  meta=(DisplayName="Get a Module's Tables and Reducers as a List of Strings",
			  		Keywords="SpacetimeDB Get Description"))
	static bool DescribeDatabase(
		const FString&DatabaseName,
		TArray<FString>& Tables,
		TArray<FString>& Reducers,
		FString& OutError);

	
};
