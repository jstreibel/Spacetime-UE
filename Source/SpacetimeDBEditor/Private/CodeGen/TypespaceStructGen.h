#pragma once
#include "SEditorViewportToolBarMenu.h"
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
	
	FString FileName;
	
	bool bPragmaOnce = true;

	struct FInclude { FString Path; bool bIsLocal; };
	
	TArray<FInclude> Includes;
	FString ApiMacro;

	void AddStruct(FStruct Struct)
	{
		FHeaderElement Element;
		Element.Type = FHeaderElement::Struct;
		Element.Index = Structs.Num();
		Element.Name = Struct.Name;
		for (const auto &Attribute : Struct.Attributes)
		{
			Element.Depends.Add(Attribute.Type);
		}
				
		HeaderElements.Add(Element);
		Structs.Add(Struct);
	}

	void AddTaggedUnion(FTaggedUnion TaggedUnion)
	{
		FHeaderElement Element;
		Element.Type = FHeaderElement::TaggedUnion;
		Element.Index = TaggedUnions.Num();
		Element.Name = "F" + TaggedUnion.BaseName;
		for (const auto &Attribute : TaggedUnion.Variants)
		{
			Element.Depends.Add(Attribute.Type);
		}
				
		HeaderElements.Add(Element);
		TaggedUnions.Add(TaggedUnion);
	}

	const auto& GetTaggedUnions() const
	{
		return TaggedUnions;
	}
	const auto& GetStructs() const
	{
		return Structs;
	}
	const TArray<FHeaderElement>& GetHeaderElements() const { return HeaderElements; }

	auto TopoSortElements() const
	{

		const int32 N = HeaderElements.Num();
		// map name -> position in In[]
		TMap<FString,int32> NameToPos;
		for (int32 i = 0; i < N; ++i)
			NameToPos.Add(HeaderElements[i].Name, i);

		// build graph: adj[u] = list of nodes that depend on u
		TArray<TArray<int32>> Adj; Adj.SetNum(N);
		TArray<int32> InDegree;    InDegree.Init(0, N);

		for (int32 u = 0; u < N; ++u)
		{
			for (auto& DepName : HeaderElements[u].Depends)
			{
				if (int32* v = NameToPos.Find(DepName))
				{
					// u depends on *v, so edge (*v)->u
					Adj[*v].Add(u);
					InDegree[u]++;
				}
			}
		}

		// collect all zero‐in‐degree nodes
		TQueue<int32> Q;
		for (int32 i = 0; i < N; ++i)
			if (InDegree[i] == 0)
				Q.Enqueue(i);

		// Kahn’s main loop
		TArray<FHeader::FHeaderElement> Sorted;
		while (!Q.IsEmpty())
		{
			int32 u; Q.Dequeue(u);
			Sorted.Add(HeaderElements[u]);
			for (int32 w : Adj[u])
			{
				if (--InDegree[w] == 0)
					Q.Enqueue(w);
			}
		}

		// if we didn’t pick up everything, there’s a cycle!
		if (Sorted.Num() == N)
		{
			UE_LOG(LogTemp, Error, TEXT("[spacetime] Cyclic dependency detected in RawModuleDef types/typespace"));
		}
		
		return Sorted;
	}

private:
	// TODO: also add Classes, Functions, etc.
	TArray<FTaggedUnion> TaggedUnions;
	TArray<FStruct> Structs;
	
	TArray<FHeaderElement> HeaderElements;
		
};

class FTypespaceStructGen
{
public:	
	static bool BuildTypesHeaders(
		const FString& ModuleName,
		const SATS::FTypespace& Typespace,
		const TArray<SATS::FExportedType>& ExportedTypes,
		FHeader &OutExported,
		FHeader &OutInline,
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
		FHeader &OutInlineHeader, FString &OutError);

	static bool GenerateNewStruct(
		const FString& ModuleName,
		const TArray<SATS::FExportedType>& ExportedTypes,
		const SATS::FOptionalString& StructName,
		const SATS::FProductType& Product,
		FStruct& OutStruct,
		FHeader &OutInlineHeader,
		FString &OutError);

public:
	static bool BuildTypespaceHeader_Deprecated(
		const FString& ModuleName,
		const FString& HeaderBaseName,
		const SATS::FTypespace& Typespace,
		const TArray<SATS::FExportedType>& ExportedTypes,
		FHeader& OutHeader,
		FString& OutError);
	
};
