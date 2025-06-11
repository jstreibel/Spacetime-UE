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

struct FAttribute
{
	FString Name;
	FString Type;
	TOptional<FString> Comment;
};

struct FTaggedUnion
{
	TArray<FString> OptionTags;

	FString BaseName={};
	TArray<FAttribute> Variants={};
	
	bool bIsReflected;
	FString SubCategory;

	TOptional<FString> Comment;
};

struct FStruct
{	
	FString Name={};
	TArray<FAttribute> Attributes={};
	
	bool bIsReflected=false;
	TArray<FString> Specifiers={};
	TMap<FString, FString> MetadataSpecifiers={};

	TOptional<FString> Comment;
};

struct FHeader
{
	bool bPragmaOnce = true;

	struct FInclude { FString Path; bool bIsLocal; };
	
	TArray<FInclude> Includes;
	FString ApiMacro;
	
	// TODO: also add Classes, Functions, etc.
	TArray<FTaggedUnion> TaggedUnions;
	TArray<FStruct> Structs;
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
