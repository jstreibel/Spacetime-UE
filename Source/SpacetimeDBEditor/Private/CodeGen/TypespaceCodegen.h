#pragma once
#include "Schema/RawModuleDefSchema.h"

struct FFunction
{
	bool bIsReflected;
	TArray<FString> Specifiers;
	TMap<FString, FString> MetadataSpecifiers;
	TMap<FString, FString> Parameters;
	FString ReturnType;
};

struct FStruct
{
	FString Name;
	TMap<FString, FString> Attributes;
	
	bool bIsReflected;
	TArray<FString> Specifiers;
	TMap<FString, FString> MetadataSpecifiers;
};

struct FHeader
{
	struct FInclude
	{
		FString Path;
		bool bIsLocal;
	};

	bool bPragmaOnce = true;
	// TODO: also add Classes, Functions, etc.
	TArray<FStruct> Structs;
	TArray<FInclude> Includes;
	FString ApiMacro;
};

class FTypespaceCodegen
{
public:
	static bool FromIntermediateRepresentation(
		const FString &HeaderBaseName,
		const SATS::FTypespace& Typespace,
		FHeader &OutHeader,
		FString &OutError);

private:
	
};
