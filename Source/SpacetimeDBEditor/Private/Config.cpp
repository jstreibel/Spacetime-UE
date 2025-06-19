#include "Config.h"

#include "Parser/Common.h"

const FString FSpacetimeConfig::ApiMacroString = "SPACETIMEDBRUNTIME_API";
const FString FSpacetimeConfig::TabString = "    ";
const FString FSpacetimeConfig::GeneratedDirectory = "StdbGenerated";

FString FSpacetimeConfig::MakeReducerCodeFileName(const FString& ModuleName)
{
	return FCommon::ToPascalCase(ModuleName) + FString(TEXT("Reducers.stdbgen"));
}

SpacetimeDB::FOptionalString FSpacetimeConfig::GetDefaultValueForType(const SpacetimeDB::EType& Type)
{
	const auto NoValue = SpacetimeDB::FOptionalString();
	
	switch (Type)
	{
	case SpacetimeDB::EType::Bool:   return FString("true");
	case SpacetimeDB::EType::I8:     return FString("0");
	case SpacetimeDB::EType::U8:     return FString("0");
	case SpacetimeDB::EType::I16:    return FString("0");
	case SpacetimeDB::EType::U16:    return FString("0");
	case SpacetimeDB::EType::I32:    return FString("0");
	case SpacetimeDB::EType::U32:    return FString("0");
	case SpacetimeDB::EType::I64:    return FString("0");
	case SpacetimeDB::EType::U64:    return FString("0");
	case SpacetimeDB::EType::I256:   return NoValue;	// type is a generated struct
	case SpacetimeDB::EType::U256:   return NoValue;	// type is a generated struct
	case SpacetimeDB::EType::F32:    return FString("0");
	case SpacetimeDB::EType::F64:    return FString("0");
	case SpacetimeDB::EType::String: return FString("\"\"");
	case SpacetimeDB::EType::Array:  return FString("{}");
	case SpacetimeDB::EType::Map:    return FString("{}");
	
	case SpacetimeDB::EType::Product:
	case SpacetimeDB::EType::Sum:		
	case SpacetimeDB::EType::Ref:		
	case SpacetimeDB::EType::Invalid:;
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

FString FSpacetimeConfig::MakeSerializationCodeFileName(const FString& ModuleName)
{
	return FCommon::ToPascalCase(ModuleName) + "Serialization.stdbgen";
}
