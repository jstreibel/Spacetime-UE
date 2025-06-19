// ModuleDefParser.h
#pragma once

#include "CoreMinimal.h"
#include "Schema/RawModuleDefSchema.h"

/**
 * Parses a RawModuleDef JSON string into schema models for tables and reducers.
 */
class SPACETIMEDBCORE_API FModuleDefParser
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
		SpacetimeDB::FRawModuleDef &RawModule,
		FString& OutError
	);

private:
	static bool ParseTypespace(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		SpacetimeDB::FTypespace& TypespaceOutput,
		FString& OutError);
	static bool ParseTypes(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		TArray<SpacetimeDB::FExportedType>& TypesOutput,
		FString& OutError);
	static bool ParseTables(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		TArray<SpacetimeDB::FTableDef>& TablesOutput,
		FString& OutError);
	static bool ParseReducers(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		TArray<SpacetimeDB::FReducerDef>& ReducersOutput,
		FString& OutError);
	static bool ParseRawModuleDef(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		SpacetimeDB::FRawModuleDef& OutDef,
		FString& OutError);
};
