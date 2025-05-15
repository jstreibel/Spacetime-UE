// UnrealStructGenerator.h
#pragma once

#include "CodeGen/IStructGenerator.h"
#include "Schema/SchemaModels.h"

/**
 * Generates Unreal-friendly USTRUCT definitions from table schema.
 */
class FUnrealStructGenerator : public IStructGenerator
{
public:
	virtual ~FUnrealStructGenerator() override = default;

	/**
	 * Generate the header code for a given table as a USTRUCT.
	 * @param Schema  Table schema containing name and fields
	 * @param OutCode Generated C++ code for the USTRUCT
	 * @return true if generation succeeded
	 */
	virtual bool Generate(
		const FTableSchema& Schema,
		FString& OutCode
	) override;
};
