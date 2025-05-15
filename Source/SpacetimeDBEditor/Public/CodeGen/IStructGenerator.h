#pragma once

#include "Schema/SchemaModels.h"

class IStructGenerator
{
public:
	virtual ~IStructGenerator() = default;
	virtual bool Generate(const FTableSchema& Schema, FString& OutCode) = 0;
};

