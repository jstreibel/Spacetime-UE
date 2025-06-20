#pragma once
#include "Schema/RawModuleDefSchema.h"


namespace SpacetimeDB
{
	struct FSpacetimeConfig
	{
		static const FString ApiMacroString;
		static const FString TabString;
		static const FString GeneratedDirectory;

		static FString MakeReducerCodeFileName(const FString& ModuleName);

		static SpacetimeDB::FOptionalString GetDefaultValueForType(const SpacetimeDB::EType& Type);
	
		static FString MakeInlineTypesCodeFileName(const FString& ModuleName);
		static FString MakeExportedTypesCodeFileName(const FString& ModuleName);
		static FString MakeSerializationCodeFileName(const FString& ModuleName);
	};
}