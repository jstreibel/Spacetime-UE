// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include <type_traits>
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


/* Provides SATS-JSON U128 support; Unreal UBT lacks uint128 reflection. */
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


using FJsonWriterRef = TSharedRef<TJsonWriter<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>>;
using FWriterFactory = TJsonWriterFactory<TCHAR,TCondensedJsonPrintPolicy<TCHAR>>;

inline void ProductStart(const FJsonWriterRef& Writer, TOptional<FString> Key)
{
	if (Key.IsSet())
	{
		const auto TagString = Key.GetValue();
		Writer->WriteArrayStart(*TagString);
	}
	else
	{
		Writer->WriteArrayStart();
	}
}

inline void ProductEnd(const FJsonWriterRef& Writer, TOptional<FString> Key)
{
	Writer->WriteArrayEnd();
}

inline void SumStart(const FJsonWriterRef& Writer, TOptional<FString> Key)
{
	if (Key.IsSet())
	{
		const auto TagString = Key.GetValue();
		Writer->WriteObjectStart(*TagString);
	}

	else
	{
		Writer->WriteObjectStart();
	}
}

inline void SumEnd(const FJsonWriterRef& Writer, const TOptional<FString>& Key)
{
	Writer->WriteObjectEnd();
}

template<typename T>
void SerializeNumberOrString(const T& NumberOrString, const FJsonWriterRef& Writer, TOptional<FString> Key = {})
{
	if (Key.IsSet())
	{
		const auto TagString = Key.GetValue();
		Writer->WriteValue(*TagString, NumberOrString);
	}
	else
	{
		Writer->WriteValue(NumberOrString);
	}
}