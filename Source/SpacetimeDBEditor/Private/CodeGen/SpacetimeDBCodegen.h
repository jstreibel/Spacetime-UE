#pragma once
#include "CoreMinimal.h"
#include "Schema/RawModuleDefSchema.h"


namespace SpacetimeDB
{
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
		static bool GenerateTablesCode(
			const FRawModuleDef& ModuleDef,
			const FString& HeaderName,
			FString& OutHeader,
			FString& OutError);

		/**
		 * Emit a Blueprint function library header + source for all reducers.
		 * @param ModuleName The module's name as present in the Spacetime server
		 * @param ModuleDef  Parsed RawModuleDef
		 * @param OutHeader  Generated Reducers.h code
		 * @param OutSource  Generated Reducers.cpp code
		 * @param OutError   Error message, in case of 'false' return value
		 */
		static bool GenerateReducersCode(
			const FString& ModuleName,
			const SpacetimeDB::FRawModuleDef& ModuleDef,
			FString& OutHeader,
			FString& OutSource,
			FString& OutError
		);

		static bool GenerateTypesSerializationCode(
			const SpacetimeDB::FRawModuleDef& ModuleDef,
			const FString& ModuleName,
			FString& OutSource,
			FString& OutHeader,
			FString& OutError);

		/**
		 * Generates headers for Typespace Products, which map to C/C++ structs.
		 * @param ModuleDef
		 * @param ModuleName 
		 * @param OutExportedTypesCode
		 * @param OutInlineTypesCode 
		 * @param OutError 
		 * @return 
		 */
		static bool GenerateTypesCode(
			const SpacetimeDB::FRawModuleDef& ModuleDef,
			const FString& ModuleName,
			FString& OutExportedTypesCode,
			FString& OutInlineTypesCode,
			FString& OutError);

	private:
		static FString ResolveAlgebraicTypeToUnrealCxx(const SpacetimeDB::FAlgebraicType& AlgebraicKind);
	};
}