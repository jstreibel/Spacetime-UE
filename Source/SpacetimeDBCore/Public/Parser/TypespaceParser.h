#pragma once
#include "Schema/RawModuleDefSchema.h"

class FTypespaceParser
{
public:
	static bool ParseTypespace(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		SpacetimeDB::FTypespace& TypespaceOutput,
		FString& OutError);
	
	static bool ParseTypes(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		TArray<SpacetimeDB::FExportedType>& TypesOutput,
		FString& OutError);
	
};
