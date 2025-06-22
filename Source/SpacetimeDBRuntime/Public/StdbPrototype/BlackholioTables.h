#pragma once

#include "CoreMinimal.h"
#include "BlackholioTables.generated.h"


USTRUCT(BlueprintType, Category="SpacetimeDB|Tables")
struct FCircleRow {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite) FString  Name;
	UPROPERTY(BlueprintReadWrite) float    Value;
	// …etc.
};
