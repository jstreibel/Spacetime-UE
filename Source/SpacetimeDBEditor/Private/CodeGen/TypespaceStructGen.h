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
	struct FAttribute
	{
		FString Name;
		FString Type;
		TOptional<FString> Comment;
	};
	
	FString Name={};
	TArray<FAttribute> Attributes={};
	
	bool bIsReflected=false;
	TArray<FString> Specifiers={};
	TMap<FString, FString> MetadataSpecifiers={};

	TOptional<FString> Comment;
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

class FTypespaceStructGen
{
public:
	static bool BuildHeaderLayoutFromIntermediateRepresentation(
		const FString& ModuleName,
		const FString& HeaderBaseName,
		const SATS::FTypespace& Typespace,
		const TArray<SATS::FExportedType>& Types,
		FHeader& OutHeader,
		FString& OutError);

private:
	
};
