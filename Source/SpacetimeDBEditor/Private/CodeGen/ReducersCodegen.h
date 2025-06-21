#pragma once
#include "TypesIRBuilder.h"


namespace SpacetimeDB
{

	class FReducersCodegen
	{
	public:
		
		static void EmitCode(
			const FString& ModuleName,
			const FRawModuleDef& ModuleDef,
			const FTypesIR& TypesIR,
			FString& OutHeader,
			FString& OutSource);

	private:
		struct FReducerParam { FString Name; FString Type; FString SerializationType; FString SerializationNameSpace; };
		
		static void EmitReducer(
			const FReducerDef& Reducer,
			const TArray<FReducerParam>& Params,
			const FString& ModuleNameNormalized,
			FString& OutHeader,
			FString& OutSource);
		static FString ResolveType(const FTypesIR& ExportedTypesIR, const FAlgebraicType& Type, const FString& ReducerName);

		static TArray<FReducerParam> BuildParamsList(
			const FString& NormalizedModuleName,
			const FTypesIR& ExportedTypesIR,
			const FReducerDef& Reducer,
			const bool bNormalizeNames);

		struct FReducerSignature { FString HeaderSignature; FString SourceSignature; };
		static FReducerSignature GenerateSignature(
			const TArray<FReducerParam>& Params,
			const FString& ReducerName,
			const FString& ClassName);
	};

	
}
