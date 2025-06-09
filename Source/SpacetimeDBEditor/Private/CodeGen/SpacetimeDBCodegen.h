#pragma once
#include "CoreMinimal.h"
#include "Schema/RawModuleDefSchema.h"

/**
 * Generates Unreal C++ code (USTRUCTs & Blueprint nodes) from SATS::RawModuleDef.
 */
class FSpacetimeDBCodeGen
{
public:
	/**
	 * Emit a single header containing all table USTRUCTs.
	 * @param ModuleDef  Parsed RawModuleDef
	 * @param HeaderName The header name, without '.h'
	 * @param OutHeader  Generated .h code
	 * @param OutError   Error, if any, description
	 */
	static bool GenerateTableStructs(
		const SATS::FRawModuleDef& ModuleDef,
		const FString& HeaderName,
		FString& OutHeader,
		FString& OutError);

	/**
	 * Emit a Blueprint function library header + source for all reducers.
	 * @param ModuleDef  Parsed RawModuleDef
	 * @param HeaderName The header name, without '.h'
	 * @param OutHeader  Generated Reducers.h code
	 * @param OutSource  Generated Reducers.cpp code
	 */
	static bool GenerateReducerFunctions(
		const SATS::FRawModuleDef& ModuleDef,
		const FString& HeaderName,
		FString& OutHeader,
		FString& OutSource,
		FString& OutError
	);

	/**
	 * Generates headers for Typespace Products, which map to C/C++ structs.
	 * @param ModuleDef 
	 * @param HeaderName 
	 * @param OutHeader 
	 * @param OutError 
	 * @return 
	 */
	static bool GenerateTypespaceStructs(
		const SATS::FRawModuleDef& ModuleDef,
		const FString& HeaderName,
		FString& OutHeader,
		FString& OutError);

private:
	static FString ResolveAlgebraicTypeToUnrealCxx(const SATS::FAlgebraicType& AlgebraicKind);
	// Map a SATS builtin (e.g. "Int", "String") to Unreal type ("int32", "FString")
	static FString MapBuiltinToUnreal(const FString& BuiltinName);
	// Sanitize identifier and convert to PascalCase
	static FString ToPascalCase(const FString& InString);
};