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
		struct FReducerParam { FString Name; FString Type; };
		
		static void EmitReducer(
			const FReducerDef& Reducer,
			const TArray<FReducerParam>& Params,
			const FString& ModuleNameNormalized,
			FString& OutHeader,
			FString& OutSource);
		
		static TArray<FReducerParam> BuildParamsList(
			const FTypesIR& ExportedTypesIR,
			const FReducerDef& Reducer);

		struct FReducerSignature { FString HeaderSignature; FString SourceSignature; };
		static FReducerSignature GenerateSignature(
			const TArray<FReducerParam>& Params,
			const FString& ReducerName,
			const FString& ClassName);
	};

	
}
