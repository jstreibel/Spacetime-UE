// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SpacetimeRuntimeSDK.generated.h"

/**
 * 
 */
UCLASS()
class SPACETIMEDBRUNTIME_API UAlgebraicSum : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
	void Serialize(FString& OutJsonPayload) const;
};

UCLASS()
class SPACETIMEDBRUNTIME_API UAlgebraicProduct : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
	void Serialize(FString& OutJsonPayload) const;
};


/**
 * 
 */
UCLASS()
class SPACETIMEDBRUNTIME_API USpacetimeRuntimeDefs : public UObject
{
	GENERATED_BODY()
};
