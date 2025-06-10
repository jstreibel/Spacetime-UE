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
	static bool ParseTypespace(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		SATS::FTypespace& TypespaceOutput,
		FString& OutError);
	static bool ParseTypes(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		TArray<SATS::FExportedType>& TypesOutput,
		FString& OutError);
	static bool ParseTables(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		TArray<SATS::FTableDef>& TablesOutput,
		FString& OutError);
	static bool ParseReducers(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		TArray<SATS::FReducerDef>& ReducersOutput,
		FString& OutError);
	static bool ParseRawModuleDef(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		SATS::FRawModuleDef& OutDef,
		FString& OutError);
};
