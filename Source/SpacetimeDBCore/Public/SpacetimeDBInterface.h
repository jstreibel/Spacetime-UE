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
	
};
