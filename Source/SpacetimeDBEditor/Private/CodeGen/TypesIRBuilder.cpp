#include "TypesIRBuilder.h"

#include "Config.h"
#include "Parser/Common.h"


namespace SpacetimeDB
{
	FString IntToString(const uint32 Value)	{ return FString::Printf(TEXT("%02d"), Value);	}

	FString GenerateNameForInlineStruct() {	static int32 Unnamed = 0; return "FProduct" + IntToString(Unnamed++); }

	FString GenerateBaseNameForInlineTaggedUnion() { static int32 Unnamed = 0; return "Sum" + IntToString(Unnamed++); }

	FString GenerateNameForAnonymousDataMember() { static int32 Unnamed = 0; return "AnonymousField_" + FString::FromInt(Unnamed++); }

	FString GetDataMemberName(const FOptionalString& Name)
	{
		if (Name.IsSet()) return Name.GetValue();
		return GenerateNameForAnonymousDataMember();
	}

	void WarnTypes(const EType Tag)
	{
		if (!IsBlueprintSupported(Tag))
		{
			const FString UEType = MapBuiltinToUnreal(TypeToString(Tag), MapToUnrealNativeRepresentation);
			const FString UETypeAlt = MapBuiltinToUnreal(TypeToString(Tag), MapToUnrealAvailableReflected);
			const FString SpacetimeBuiltIn = TypeToString(Tag);

			UE_LOG(LogTemp, Warning,
				TEXT("[SpacetimeDB] Mapping Spacetime type '%s' to Unreal '%s'; "
				"Unreal lacks native '%s' Blueprint support"), *SpacetimeBuiltIn, *UETypeAlt, *UEType);

			return;
		}
			
		if (Tag == EType::Array || Tag == EType::Map)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SpacetimeDB] SATS-JSON BuiltIn types 'Array' and 'Map' "
				"not currently implemented in Unreal codegen"))
		}
	}

	void BuildElementList(
		const TArray<FTaggedUnion>& TaggedUnions,
		const TArray<FStruct>& Structs,
		TArray<FTypesIR::FHeaderElement>& OutElements)
	{
		// TaggedUnions (Algebraic Sums) first
		for (int32 i = 0; i < TaggedUnions.Num(); ++i)
		{
			const auto& TU = TaggedUnions[i];
			FTypesIR::FHeaderElement E;
			E.Type  = FTypesIR::FHeaderElement::TaggedUnion;
			E.Index = i;
			E.Name  = TU.Name;
			// collect any variant‐types that refer to other elements:
			for (auto& Attr : TU.Variants)
				E.Depends.Add(Attr.Type);
			OutElements.Add(MoveTemp(E));
		}

		// then Structs (Algebraic Products)
		for (int32 i = 0; i < Structs.Num(); ++i)
		{
			const auto& S  = Structs[i];
			FTypesIR::FHeaderElement E;
			E.Type  = FTypesIR::FHeaderElement::Struct;
			E.Index = i;
			E.Name  = S.Name;
			for (auto& Attr : S.DataMembers)
				E.Depends.Add(Attr.Type);
			OutElements.Add(MoveTemp(E));
		}
	}

	FStruct FTypespaceStructIRBuilder::GenerateNewStruct(
		const FString& ModuleName,
		const TArray<FExportedType>& ExportedTypes,
		const FString& StructName,
		const FProductType& ProductOrigin,
		FTypesIR& OutInlineHeader)
	{
		FStruct OutStruct;
		
		OutStruct.Name = StructName;
		OutStruct.ProductOrigin = ProductOrigin;

		const auto UnrealFormattedModuleName = FCommon::ToPascalCase(ModuleName);
	
		OutStruct.bIsReflected = true;
		OutStruct.Specifiers.Add("BlueprintType");
		OutStruct.MetadataSpecifiers.Add("Category", "\"SpacetimeDB|" + UnrealFormattedModuleName + "\"");

		UE_LOG(LogTemp, Log, TEXT("[SpacetimeDB] Generating Struct: %s"), *OutStruct.Name);
		for (const auto& [DataMemberOptionalName, DataMemberAlgebraicType] : ProductOrigin.Elements)
		{		
			const auto RawName = GetDataMemberName(DataMemberOptionalName);
			const auto AlgebraicKind = DataMemberAlgebraicType->Type;

			FDataMember DataMember(DataMemberAlgebraicType);
			DataMember.Name = FCommon::ToPascalCase(RawName);
		
			if (AlgebraicKind == EType::Product)
			{
				
				const auto DataMemberType = "F" + DataMember.Name;
			
				const auto &ProductElement = DataMemberAlgebraicType->Product;
				
				FStruct NewStruct =
				GenerateNewStruct(ModuleName, ExportedTypes, DataMemberType, ProductElement, OutInlineHeader);
				
				DataMember.Type = DataMemberType;
				DataMember.Comment = RawName + ": Product";
				OutInlineHeader.AddStruct(NewStruct);
			}

			else if (AlgebraicKind == EType::Sum)
			{			
				const auto &SumElement = DataMemberAlgebraicType->Sum;
				
				FTaggedUnion NewTaggedUnion =
				GenerateNewTaggedUnion(
					ModuleName, ExportedTypes,
					DataMember.Name,  SumElement,OutInlineHeader);
				
				DataMember.Type = "F" + DataMember.Name;
				DataMember.Comment = RawName + ": Sum";
				OutInlineHeader.AddTaggedUnion(NewTaggedUnion);
			}

			else if (AlgebraicKind == SpacetimeDB::EType::Ref)
			{
				const auto Index = DataMemberAlgebraicType->Ref.Index;
				const auto& Referenced = ExportedTypes[Index];
				
				DataMember.Type = FCommon::MakeStructName(Referenced.Name.Name, ModuleName);
				DataMember.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(AlgebraicKind);
				DataMember.Comment = RawName + ": " + Referenced.Name.Name;
			}

			else if (IsBuiltIn(AlgebraicKind))
			{
				DataMember.Type = MapBuiltinToUnreal(SpacetimeDB::TypeToString(AlgebraicKind), MapToUnrealAvailableReflected);
				DataMember.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(AlgebraicKind);
				DataMember.Comment = RawName + ": " + TypeToString(AlgebraicKind);
				
				WarnTypes(AlgebraicKind);
			}

			else if (AlgebraicKind == EType::Invalid)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid SATS-JSON type found in Typespace codegen"));
				DataMember.Type = "// <invalid type> ";
			}

			else
			{
				UE_LOG(LogTemp, Error, TEXT("Internal inconsistency found in Typespace codegen - unhandled SATS-JSON type"));
				DataMember.Type = "// <unhandled type> ";
			}

			OutStruct.DataMembers.Add(DataMember);		
		}

		return OutStruct;
	}

	FTaggedUnion FTypespaceStructIRBuilder::GenerateNewTaggedUnion(
		const FString& ModuleName,
		const TArray<FExportedType>& ExportedTypes,
		const FString& UnionName,
		const FSumType& SumOrigin,
		FTypesIR &OutInlineHeader)
	{
		FTaggedUnion OutTaggedUnion;
		
		// OutTaggedUnion.BaseName = UnionName.IsSet() ? UnionName.GetValue() : GenerateBaseNameForInlineTaggedUnion();
		OutTaggedUnion.Name = UnionName;
		OutTaggedUnion.SumOrigin = SumOrigin;

		const auto UnrealFormattedModuleName = FCommon::ToPascalCase(ModuleName);
	
		OutTaggedUnion.bIsReflected = true;
		OutTaggedUnion.SubCategory = UnrealFormattedModuleName;

		// const auto Anonymous = SATS::FOptionalString();
	
		UE_LOG(LogTemp, Log, TEXT("[SpacetimeDB] Generating Tagged Union: F%s"), *OutTaggedUnion.Name);
		for (const auto& [VariantOptionalName, VariantAlgebraicType] : SumOrigin.Options)
		{		
			const auto RawName = GetDataMemberName(VariantOptionalName);
			const auto Tag = VariantAlgebraicType->Type;

			FDataMember DataMember(VariantAlgebraicType);
			DataMember.Name = FCommon::ToPascalCase(RawName);
			
			if (Tag == EType::Product)
			{
				const auto DataMemberType = "F" + DataMember.Name;
				
				FStruct NewStruct =
					GenerateNewStruct(ModuleName, ExportedTypes, DataMemberType, VariantAlgebraicType->Product, OutInlineHeader);
				OutInlineHeader.AddStruct(NewStruct);

				DataMember.Type = DataMemberType;
				OutTaggedUnion.OptionTags.Add(NewStruct.Name);
			}

			else if (Tag == EType::Sum)
			{				
				auto NewTaggedUnion =
					GenerateNewTaggedUnion(ModuleName, ExportedTypes, DataMember.Name, VariantAlgebraicType->Sum, OutInlineHeader);
				OutInlineHeader.AddTaggedUnion(NewTaggedUnion);

				DataMember.Type = "F" + DataMember.Name;
				OutTaggedUnion.OptionTags.Add(DataMember.Name);
			}

			else if (Tag == EType::Ref)
			{
				const auto Index = VariantAlgebraicType->Ref.Index;
				const auto& Referenced = ExportedTypes[Index];
				const FString Type = Referenced.Name.Name;
				
				DataMember.Type = FCommon::MakeStructName(Referenced.Name.Name, ModuleName);
				DataMember.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(Tag);
				DataMember.Comment = RawName + ": " + Referenced.Name.Name;
			}

			else if (IsBuiltIn(Tag))
			{
				DataMember.Type = MapBuiltinToUnreal(TypeToString(Tag), MapToUnrealAvailableReflected);
				DataMember.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(Tag);
				DataMember.Comment = RawName + ": " + TypeToString(Tag);
				
				WarnTypes(Tag);
			}

			else if (Tag == EType::Invalid)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid SATS-JSON type found in Typespace codegen"));
				DataMember.Type = "// <invalid type> ";
			}

			else
			{
				UE_LOG(LogTemp, Error, TEXT("Internal inconsistency found in Typespace codegen - unhandled SATS-JSON type"));
				DataMember.Type = "// <unhandled type> ";
			}

			OutTaggedUnion.Variants.Add(DataMember);		
		}

		return OutTaggedUnion;
	}

	bool FTypespaceStructIRBuilder::BuildTypesIR(
		const FString& ModuleName,
		const FTypespace& Typespace,
		const TArray<FExportedType>& ExportedTypesIn,
		FTypesIR &OutExported,
		FTypesIR &OutInline,
		FString& OutError)
	{
		// TODO: check if 'ExportedTypes' have properly matched Refs in 'Typespace'

		// We want exported types to be in the same order as they appear in the JSON RawModuleDef file's
		// 'typespace' section.
		const auto ExportedTypes = SortExportedTypesByRef(ExportedTypesIn);
	
		const FString UnrealFormattedModuleName = FCommon::ToPascalCase(ModuleName);
		const FString ExportedTypesHeaderName = FSpacetimeConfig::MakeExportedTypesCodeFileName(ModuleName);
		const FString InlineTypesHeaderName = FSpacetimeConfig::MakeInlineTypesCodeFileName(ModuleName);

		OutInline.ApiMacro = FSpacetimeConfig::ApiMacroString;
		OutInline.Includes.Add({"CoreMinimal.h", true});
		OutInline.Includes.Add({"SpacetimeRuntimeSDK.h", true});
		OutInline.Includes.Add({InlineTypesHeaderName + ".generated.h", true});

		OutExported.ApiMacro = FSpacetimeConfig::ApiMacroString;
		OutExported.Includes.Add({"CoreMinimal.h", true});
		OutExported.Includes.Add({InlineTypesHeaderName + ".h", true});
		OutExported.Includes.Add({ExportedTypesHeaderName + ".generated.h", true});

		for (const auto& Type : ExportedTypes)
		{
			const auto Index = Type.TypeRef;
			const auto &AlgebraicType = Typespace.TypeEntries[Index];

			if (AlgebraicType.Type != SpacetimeDB::EType::Product)
			{
				OutError = FString::Printf(TEXT("Header generation for types in 'typespace' other than C++ structs "
					"(i.e. 'Product' Sats-Type) not implemented - problem occured with type '%i'"), Index);
				return false;
			}

			FString StructName = FCommon::MakeStructName(Type.Name.Name, ModuleName);
			
			FStruct Struct =
				GenerateNewStruct(ModuleName, ExportedTypes, StructName, AlgebraicType.Product, OutInline);
			Struct.MetadataSpecifiers.Add("Category", "\"SpacetimeDB|" + UnrealFormattedModuleName + "\"");
			Struct.bIsExportedType = true;
			Struct.TypespaceIndex = Index;
			
			OutExported.AddStruct(Struct);
		}
		
		return true;
	}
}