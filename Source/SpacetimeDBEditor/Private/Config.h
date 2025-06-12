#pragma once
#include "Schema/RawModuleDefSchema.h"

struct FSpacetimeConfig
{
	static const FString ApiMacroString;
	static const FString TabString;
	static const FString GeneratedDirectory;

	static FString MakeReducerCodeFileName(const FString& ModuleName);

	static SATS::FOptionalString GetDefaultValueForType(const SATS::EType& Type);
	
	static FString MakeInlineTypesCodeFileName(const FString& ModuleName);
	static FString MakeExportedTypesCodeFileName(const FString& ModuleName);
};
