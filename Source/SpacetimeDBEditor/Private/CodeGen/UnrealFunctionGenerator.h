#pragma once
#include "CodeGen/IFunctionGenerator.h"
#include "Schema/SchemaModels.h"

class FUnrealFunctionGenerator : public IFunctionGenerator
{
public:
	virtual ~FUnrealFunctionGenerator() override = default;

	/**
	 * Generate the header code for a given reducer as a static UFUNCTION.
	 * @param Schema    Reducer schema containing name and parameters
	 * @param OutCode   Generated C++ code for the UFUNCTION stub
	 * @return true if generation succeeded
	 */
	virtual bool Generate(
		const FReducerSchema& Schema,
		FString& OutCode
	) override;
};
