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

	/*
	   These are used to turn struct dependencies directed graph
	   into a topologically sorted TArray of Header Elements
	   (Structs, TaggedUnions, etc.)
	 */
	struct FHeaderElement
	{
		enum EType { TaggedUnion, Struct } Type;
		int32 Index;				// index into the corresponding array
		FString Name;				// union BaseName or struct Name
		TArray<FString> Depends;	// Names of other elements this one references
	};
	TArray<FHeaderElement> HeaderElements;
		
};

class FTypespaceStructGen
{
public:
	static bool BuildExportedTypesHeader(
		const FString& ModuleName,
		const FString& HeaderBaseName,
		const SATS::FTypespace& Typespace,
		const TArray<SATS::FExportedType>& Types,
		FHeader& OutHeader,
		FString& OutError);
private:
	/*
	 * Builds a topologically sorted header elements list
	 */
	static bool BuildAndSortElementList(
		const TArray<FTaggedUnion>& TaggedUnions,
		const TArray<FStruct>& Structs,
		TArray<FHeader::FHeaderElement>& OutElements,
		FString& OutError);
	
	static bool GenerateNewTaggedUnion(
		const FString& ModuleName,
		const TArray<SATS::FExportedType>& ExportedTypes,
		const SATS::FOptionalString& UnionName,
		const SATS::FSumType& Sum,
		FTaggedUnion& OutTaggedUnion,
		FHeader &OutHeader, FString &OutError);

	static bool GenerateNewStruct(
		const FString& ModuleName,
		const TArray<SATS::FExportedType>& ExportedTypes,
		const SATS::FOptionalString& StructName,
		const SATS::FProductType& Product,
		FStruct& OutStruct,
		FHeader &OutHeader,
		FString &OutError);
	
};
