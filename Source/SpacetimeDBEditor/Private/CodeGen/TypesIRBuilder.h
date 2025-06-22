#pragma once

#include "Schema/RawModuleDefSchema.h"


namespace SpacetimeDB
{
	struct FDataMember
	{
		TSharedRef<FAlgebraicType> Origin;
		
		FString Name;					 // the normalized datamember name
		FString Type;					 // the normalized type name
		
		FString BaseName;				 // either the original name as appears in RawModuleDef, or a unique generated name if anonymous
		
		TOptional<FString> DefaultValue;
		TOptional<FString> Comment;


		FDataMember() = delete;
		FDataMember(const FDataMember& Other) = default;
		FDataMember(FDataMember&& Other) = default;
		FDataMember& operator=(const FDataMember& Other) = default;
		FDataMember& operator=(FDataMember&& Other) = default;
		~FDataMember() = default;

		explicit FDataMember(const TSharedRef<FAlgebraicType>& Origin) : Origin(Origin) {}

		auto operator == (const FDataMember& DataMember) const
		{
			return Name == DataMember.Name
				&& Type == DataMember.Type;
		}
	};


	struct FTaggedUnion
	{
		FSumType SumOrigin;

		FString Name={};
		TArray<FDataMember> Variants={};
	
		bool bIsReflected;
		FString SubCategory;

		TOptional<FString> Comment;
		bool operator==(const FTaggedUnion& TaggedUnion) const
		{
			return Name   == TaggedUnion.Name
				&& Variants   == TaggedUnion.Variants;
		};
	};


	struct FStruct
	{		
		FProductType ProductOrigin;
	
		FString Name={};
		TArray<FDataMember> DataMembers={};
	
		bool bIsReflected=false;
		TArray<FString> Specifiers={};
		TMap<FString, FString> MetadataSpecifiers={};

		TOptional<FString> Comment;
		auto operator == (const FStruct& Struct) const
		{
			return Name		   == Struct.Name
				&& DataMembers == Struct.DataMembers;
		}

		// Equivalent to being top-level in 'typespace' (TODO: is it?)
		// Note: if this is equivalent to being top-level, then we can remove the bool and leave only the optional below
		// Note (2): if we leave only the optional, perhaps we should hide this from the user, and expose via
		// getters and setters?
		bool bIsExportedType = false;
		TOptional<uint8> TypespaceIndex;
	};


	struct FTypesIR
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

		/**
		 * Adds a structure to the header if it is not already present.
		 * The structure is added along with its dependencies and is represented
		 * as a header element in a topologically sorted collection for further processing.
		 *
		 * @param Struct The structure to be added. This includes its name, data members, and metadata.
		 */
		void AddStruct(FStruct Struct)
		{
			for (const auto &MemberStruct : Structs)
			{
				if (MemberStruct == Struct) return;
			}
		
			FHeaderElement Element;
			Element.Type = FHeaderElement::Struct;
			Element.Index = Structs.Num();
			Element.Name = Struct.Name;
			for (const auto &Attribute : Struct.DataMembers)
			{
				Element.Depends.Add(Attribute.Type);
			}
				
			HeaderElements.Add(Element);
			Structs.Add(Struct);
		}

		void AddTaggedUnion(FTaggedUnion TaggedUnion)
		{
			for (const auto &MemberUnion : TaggedUnions)
			{
				if (MemberUnion == TaggedUnion) return;
			}
		
			FHeaderElement Element;
			Element.Type = FHeaderElement::TaggedUnion;
			Element.Index = TaggedUnions.Num();
			Element.Name = TaggedUnion.Name;
			for (const auto &Attribute : TaggedUnion.Variants)
			{
				Element.Depends.Add(Attribute.Type);
			}
				
			HeaderElements.Add(Element);
			TaggedUnions.Add(TaggedUnion);
		}

		const auto& GetTaggedUnions() const{ return TaggedUnions; }
		const auto& GetStructs() const{ return Structs; }
		const TArray<FHeaderElement>& GetAllElements() const { return HeaderElements; }

		auto GetTopoSortedElements() const
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
			TArray<FTypesIR::FHeaderElement> Sorted;
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
			if (Sorted.Num() != N)
			{
				UE_LOG(LogTemp, Error, TEXT("[SpacetimeDB] Cyclic dependency detected in RawModuleDef types/typespace"));
			}
		
			return Sorted;
		}

	private:
		// TODO: also add Classes, Functions, etc.
		TArray<FTaggedUnion> TaggedUnions;
		TArray<FStruct> Structs;
	
		TArray<FHeaderElement> HeaderElements;
		
	};


	class FTypespaceStructIRBuilder
	{
		
	public:
		
		static bool BuildTypesIR(
			const FString& ModuleName,
			const FTypespace& Typespace,
			const TArray<FExportedType>& ExportedTypesIn,
			FTypesIR &OutExported,
			FTypesIR &OutInline,
			FString &OutError);

	private:

		static FDataMember MakeDataMemberFromField(
			const TOptional<FString>& OriginalName,
			const TSharedRef<FAlgebraicType>& AlgebraicType,
			const TArray<FExportedType>& ExportedTypes,
			const FString& ModuleName,
			FTypesIR& OutInlineHeader);
		
		static FStruct MakeStruct(
			const FString& ModuleName,
			const TArray<FExportedType>& ExportedTypes,
			const FString& StructName,
			const FProductType& ProductOrigin,
			FTypesIR &OutInlineHeader);

		static FTaggedUnion MakeTaggedUnion(
			const FString& ModuleName,
			const TArray<FExportedType>& ExportedTypes,
			const FString& UnionName,
			const FSumType& SumOrigin,
			FTypesIR &OutInlineHeader);
	
	};
}