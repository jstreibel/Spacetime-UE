#pragma once
#include "CoreMinimal.h"
#include "TypesIRBuilder.h"
#include "Schema/RawModuleDefSchema.h"


namespace SpacetimeDB
{
	/**
	 * Generates Unreal C++ code (USTRUCTs & Blueprint nodes) from SATS::RawModuleDef.
	 */
	class FCodeGen
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
		
		static bool GenerateReducersCode(
			const FString& ModuleName,
			const FRawModuleDef& ModuleDef,
			const FTypesIR& ExportedTypesIR,
			FString& OutHeader,
			FString& OutSource,
			FString& OutError);

		static bool GenerateTypesSerializationCode(
			const FTypesIR& ExportedTypesIR,
			const FTypesIR& InlineTypesIR,
			const FString& ModuleName,
			FString& OutSource,
			FString& OutHeader,
			FString& OutError);

		/**
		 * Generates headers for Typespace Products and Sums (which map to C/C++ structs).
		 * @param ExportedTypesIR
		 * @param InlineTypesIR 
		 * @param OutExportedTypesCode
		 * @param OutInlineTypesCode 
		 * @param OutError 
		 * @return 
		 */
		static bool GenerateTypesCode(
			const FTypesIR& ExportedTypesIR,
			const FTypesIR& InlineTypesIR,
			FString& OutExportedTypesCode,
			FString& OutInlineTypesCode,
			FString& OutError);

	private:
		static FString ResolveAlgebraicTypeToUnrealCxx(const SpacetimeDB::FAlgebraicType& AlgebraicKind);
	};
}