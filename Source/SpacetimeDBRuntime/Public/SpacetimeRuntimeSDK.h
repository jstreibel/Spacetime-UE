// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SpacetimeRuntimeSDK.generated.h"


// Sum base
USTRUCT(BlueprintType)
struct FSpacetimeSum
{
	GENERATED_BODY()
};


// Product base
USTRUCT(BlueprintType)
struct FSpacetimeProduct
{
	GENERATED_BODY()
};


/* Provides SATS-JSON U256 support; Unreal UBT lacks uint256 reflection. */
USTRUCT(BlueprintType, Category="SpacetimeDB")
struct SPACETIMEDBRUNTIME_API FUInt128 : public FSpacetimeProduct {

	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FString Value;

};


/* Provides SATS-JSON U256 support; Unreal UBT lacks uint256 reflection. */
USTRUCT(BlueprintType, Category="SpacetimeDB")
struct SPACETIMEDBRUNTIME_API FUInt256 : public FSpacetimeProduct {

	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FString Value;

};


/* Provides SATS-JSON I256 support; Unreal UBT lacks int256 reflection. */
USTRUCT(BlueprintType, Category="SpacetimeDB")
struct SPACETIMEDBRUNTIME_API FInt256 : public FSpacetimeProduct {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Value;

};


/**
 * 
 */
UCLASS()
class SPACETIMEDBRUNTIME_API USpacetimeSerialization : public UObject
{
	GENERATED_BODY()

public:
	// UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
	// static FString SerializeToJson(const UScriptStruct* SpacetimeType, FString &JsonPayload);
	
};
