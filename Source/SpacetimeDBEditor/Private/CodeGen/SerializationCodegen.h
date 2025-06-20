#pragma once

#include "TypesIRBuilder.h"


namespace SpacetimeDB
{
	class FSerializationCodegen
	{
		
	public:
		static void EmitCode(
			const FTypesIR& ExportedTypesIR,
			const FTypesIR& InlineTypesIR,
			const FString& ModuleName,
			FString& OutHeader,
			FString& OutSource);

	private:

		static void EmitTypesIRCode(
			const FTypesIR& TypesIR,
			FString& OutHeader,
			FString& OutSource);

		static void GenerateSumSerializationCode(
			const FTaggedUnion& Sum,
			FString& OutHeader,
			FString& OutSource);
		
		static void GenerateProductSerializationCode(
			const FStruct& Struct,
			FString& OutHeader,
			FString& OutSource);

		static void EmitFunctionCall(
			const FDataMember& ToType,
			FString& OutSource,
			TOptional<FString> Tag = {},
			const int Indent = 1);
	
	};
}