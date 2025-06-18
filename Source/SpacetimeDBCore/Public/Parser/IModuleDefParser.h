#pragma once
#include "Schema/SchemaModels.h"

class IModuleDefParser
{
public:
	virtual ~IModuleDefParser() = default;
	virtual bool Parse(const FString& RawJson,
					   TArray<FTableSchema>& OutTables,
					   TArray<FReducerSchema>& OutReducers,
					   FString& OutError) = 0;
};

