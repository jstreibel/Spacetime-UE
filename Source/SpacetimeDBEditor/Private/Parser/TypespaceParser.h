#pragma once
#include "Schema/RawModuleDefSchema.h"

class FTypespaceParser
{
public:
	static bool ParseTypespace(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		SATS::FTypespace& TypespaceOutput,
		FString& OutError);

	
};
