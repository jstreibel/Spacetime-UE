// ModuleDefParser.h
#pragma once

#include "CoreMinimal.h"
#include "Schema/SchemaModels.h"

/**
 * Parses a RawModuleDef JSON string into schema models for tables and reducers.
 */
class FModuleDefParser
{
public:
	/**
	 * Parses the given JSON string.
	 * @param RawJson      JSON from `spacetime describe --json`
	 * @param OutTables    Parsed table schemas
	 * @param OutReducers  Parsed reducer schemas
	 * @param OutError     Error message on failure
	 * @return true on successful parse
	 */
	static bool Parse(
		const FString& RawJson,
		TArray<FTableSchema>& OutTables,
		TArray<FReducerSchema>& OutReducers,
		FString& OutError
	);
};