#include "Config.h"

#include "Parser/Common.h"

const FString FSpacetimeConfig::ApiMacroString = "SPACETIMEDBRUNTIME_API";
const FString FSpacetimeConfig::TabString = "    ";
const FString FSpacetimeConfig::GeneratedDirectory = "StdbGenerated";

FString FSpacetimeConfig::MakeReducerCodeFileName(const FString& ModuleName)
{
	return FCommon::ToPascalCase(ModuleName) + FString(TEXT("Reducers.stdbgen"));
}

SATS::FOptionalString FSpacetimeConfig::GetDefaultValueForType(const SATS::EType& Type)
{
	const auto NoValue = SATS::FOptionalString();
	
	switch (Type)
	{
	case SATS::EType::Bool:   return FString("true");
	case SATS::EType::I8:     return FString("0");
	case SATS::EType::U8:     return FString("0");
	case SATS::EType::I16:    return FString("0");
	case SATS::EType::U16:    return FString("0");
	case SATS::EType::I32:    return FString("0");
	case SATS::EType::U32:    return FString("0");
	case SATS::EType::I64:    return FString("0");
	case SATS::EType::U64:    return FString("0");
	case SATS::EType::I256:   return NoValue;	// type is a generated struct
	case SATS::EType::U256:   return NoValue;	// type is a generated struct
	case SATS::EType::F32:    return FString("0");
	case SATS::EType::F64:    return FString("0");
	case SATS::EType::String: return FString("\"\"");
	case SATS::EType::Array:  return FString("{}");
	case SATS::EType::Map:    return FString("{}");
	
	case SATS::EType::Product:
	case SATS::EType::Sum:		
	case SATS::EType::Ref:		
	case SATS::EType::Invalid:;
	}

	return NoValue;
}

FString FSpacetimeConfig::MakeInlineTypesCodeFileName(const FString& ModuleName)
{
	return FCommon::ToPascalCase(ModuleName) + "InlineTypes.stdbgen";	
}

FString FSpacetimeConfig::MakeExportedTypesCodeFileName(const FString& ModuleName)
{
	return FCommon::ToPascalCase(ModuleName) + "ExportedTypes.stdbgen";
}
