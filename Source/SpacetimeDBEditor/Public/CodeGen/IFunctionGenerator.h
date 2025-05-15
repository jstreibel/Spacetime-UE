#pragma once

#include "Schema/SchemaModels.h"

class IFunctionGenerator
{
public:
	virtual ~IFunctionGenerator() = default;
	virtual bool Generate(const FReducerSchema& Schema, FString& OutCode) = 0;
};
