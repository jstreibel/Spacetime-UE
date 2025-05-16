// ModuleDefParser.h
#pragma once

#include "CoreMinimal.h"
#include "Schema/RawModuleDefSchema.h"

/**
 * Parses a RawModuleDef JSON string into schema models for tables and reducers.
 */
class FModuleDefParser
{
public:
	/**
	 * Parses the given JSON string.
	 * @param RawJson      JSON from `spacetime describe --json`
	 * @param RawModule    Parsed spacetime module schemas
	 * @param OutError     Error message on failure
	 * @return true on successful parse
	 */
	static bool Parse(
		const FString& RawJson,
		SATS::FRawModuleDef &RawModule,
		FString& OutError
	);

private:

	static bool ParseRawModuleDef(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		SATS::FRawModuleDef& OutDef,
		FString& OutError);

	static bool ParseTypespaceProductType(
		const TArray<TSharedPtr<FJsonValue>> &ProductTermsArray,
		SATS::FProductType& OutProduct,
		FString& OutError);

	static bool ParseTypespaceSumType(
		const TArray<TSharedPtr<FJsonValue>> &SumTerms,
		SATS::FSumType& OutSum,
		FString& OutError); 
};
