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
	FString Name={};
	TMap<FString, FString> Attributes={};
	
	bool bIsReflected=false;
	TArray<FString> Specifiers={};
	TMap<FString, FString> MetadataSpecifiers={};
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
	TArray<TSharedPtr<FStruct>> Structs;
	TArray<FInclude> Includes;
	FString ApiMacro;

	bool AnyStructReflected() const
	{
		for (const auto& Struct : Structs)
		{
			if (Struct->bIsReflected) return true;
		}

		return false;
	}
};

class FTypespaceCodegen
{
public:
	static bool BuildHeaderLayoutFromIntermediateRepresentation(
		const FString& HeaderBaseName,
		const SATS::FTypespace& Typespace,
		const TArray<SATS::FExportedType>& Types,
		FHeader& OutHeader,
		FString& OutError);

private:
	
};
