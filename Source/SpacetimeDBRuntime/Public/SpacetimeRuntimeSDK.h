// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SpacetimeRuntimeSDK.generated.h"


/* Provides SATS-JSON U256 support; Unreal UBT lacks uint256 reflection. */
USTRUCT(BlueprintType, Category="SpacetimeDB")
struct SPACETIMEDBRUNTIME_API FUInt256 {

	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FString Value;

};


/* Provides SATS-JSON I256 support; Unreal UBT lacks int256 reflection. */
USTRUCT(BlueprintType, Category="SpacetimeDB")
struct SPACETIMEDBRUNTIME_API FInt256 {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Value;

};


/**
 * 
 */
UCLASS()
class SPACETIMEDBRUNTIME_API USpacetimeRuntimeDefs : public UObject
{
	GENERATED_BODY()
};
