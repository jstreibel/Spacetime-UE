#include "TypesHeadersIRBuilder.h"

#include "Config.h"
#include "Parser/Common.h"

FString IntToString(const uint32 Value)
{
	return FString::Printf(TEXT("%02d"), Value);
}

FString GenerateNameForInlineStruct()
{
	static int32 Unnamed = 0;

	return "FProduct" + IntToString(Unnamed++);
}

FString GenerateBaseNameForInlineTaggedUnion()
{
	static int32 Unnamed = 0;

	return "Sum" + IntToString(Unnamed++);
}

FString GenerateNameForAnonymousDataMember()
{
	static int32 Unnamed = 0;

	return "AnonymousField_" + FString::FromInt(Unnamed++);
}

FString GetDataMemberName(const SATS::FOptionalString& Name)
{
	if (Name.IsSet()) return Name.GetValue();

	return GenerateNameForAnonymousDataMember();
}

void WarnTypes(const SATS::EType Tag)
{
	if (!SATS::IsReflectedInUnreal(Tag))
	{
		const FString UEType = SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag), true);
		const FString UETypeAlt = SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag), false);
		const FString SpacetimeBuiltIn = SATS::TypeToString(Tag);

		UE_LOG(LogTemp, Warning,
			TEXT("[spacetime] Mapping Spacetime type '%s' to Unreal '%s'; "
			"Unreal lacks native '%s' Blueprint support"), *SpacetimeBuiltIn, *UETypeAlt, *UEType);

		return;
	}
			
	if (Tag == SATS::EType::Array || Tag == SATS::EType::Map)
	{
		UE_LOG(LogTemp, Warning, TEXT("[spacetime] SATS-JSON BuiltIn types 'Array' and 'Map' "
			"not currently implemented in Unreal codegen"))
	}
}

void AddMissingBuiltIns(FHeader& Header)
{
	{
		FStruct UInt256;
		UInt256.Name = TEXT("FUInt256");
		UInt256.DataMembers.Add({"Value", "FString"});
		UInt256.bIsReflected = true;
		UInt256.Specifiers.Add("BlueprintType");
		UInt256.MetadataSpecifiers.Add("Category", "\"SpacetimeDB\"");
		UInt256.Comment = TEXT("Provides SATS-JSON U256 support; Unreal UBT lacks uint256 reflection.");

		Header.AddStruct(UInt256);
	}

	{
		FStruct Int256;
		Int256.Name = TEXT("FInt256");
		Int256.DataMembers.Add({"Value", "FString"});
		Int256.bIsReflected = true;
		Int256.Specifiers.Add("BlueprintType");
		Int256.MetadataSpecifiers.Add("Category", "\"SpacetimeDB\"");
		Int256.Comment = TEXT("Provides SATS-JSON I256 support; Unreal UBT lacks int256 reflection.");
		
		Header.AddStruct(Int256);
	}
}

void BuildElementList(
	const TArray<FTaggedUnion>& TaggedUnions,
	const TArray<FStruct>& Structs,
	TArray<FHeader::FHeaderElement>& OutElements)
{
	// TaggedUnions first
	for (int32 i = 0; i < TaggedUnions.Num(); ++i)
	{
		const auto& TU = TaggedUnions[i];
		FHeader::FHeaderElement E;
		E.Type  = FHeader::FHeaderElement::TaggedUnion;
		E.Index = i;
		E.Name  = TU.Name;
		// collect any variant‐types that refer to other elements:
		for (auto& Attr : TU.Variants)
			E.Depends.Add(Attr.Type);
		OutElements.Add(MoveTemp(E));
	}

	// then Structs
	for (int32 i = 0; i < Structs.Num(); ++i)
	{
		const auto& S  = Structs[i];
		FHeader::FHeaderElement E;
		E.Type  = FHeader::FHeaderElement::Struct;
		E.Index = i;
		E.Name  = S.Name;
		for (auto& Attr : S.DataMembers)
			E.Depends.Add(Attr.Type);
		OutElements.Add(MoveTemp(E));
	}
}

bool FTypespaceStructIRBuilder::GenerateNewStruct(
	const FString& ModuleName,
	const TArray<SATS::FExportedType>& ExportedTypes,
	const FString& StructName,
	const SATS::FProductType& Product,
	FStruct& OutStruct,
	FHeader& OutInlineHeader,
	FString& OutError)
{	
	OutStruct.Name = StructName;

	const auto UnrealFormattedModuleName = FCommon::ToPascalCase(ModuleName);
	
	OutStruct.bIsReflected = true;
	OutStruct.Specifiers.Add("BlueprintType");
	OutStruct.MetadataSpecifiers.Add("Category", "\"SpacetimeDB|" + UnrealFormattedModuleName + "\"");

	UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating Struct: %s"), *OutStruct.Name);
	for (const auto& [DataMemberOptionalName, DataMemberAlgebraicType] : Product.Elements)
	{
		if (!DataMemberAlgebraicType.IsValid())
		{
			OutError = FString::Printf(TEXT("invalid pointer while generating struct %s"), *OutStruct.Name);
			return false;
		}
		
		const auto RawName = GetDataMemberName(DataMemberOptionalName);
		const auto AlgebraicKind = DataMemberAlgebraicType->Type;
		
		if (AlgebraicKind == SATS::EType::Product)
		{
			const auto DataMemberName = FCommon::ToPascalCase(RawName);
			const auto DataMemberType = "F" + DataMemberName;
			
			const auto &ProductElement = DataMemberAlgebraicType->Product;
			FStruct NewStruct;
			if (!GenerateNewStruct(ModuleName, ExportedTypes, DataMemberType, ProductElement, NewStruct, OutInlineHeader, OutError))
			{
				return false;
			}
			OutStruct.DataMembers.Add({DataMemberName, DataMemberType});
			
			OutInlineHeader.AddStruct(NewStruct);

			continue;
		}

		if (AlgebraicKind == SATS::EType::Sum)
		{
			const auto DataMemberName = FCommon::ToPascalCase(RawName);
			const auto DataMemberType = DataMemberName;
			
			const auto &SumElement = DataMemberAlgebraicType->Sum;
			FTaggedUnion NewTaggedUnion;
			if (!GenerateNewTaggedUnion(
					ModuleName, ExportedTypes,
					DataMemberType,  SumElement,
					NewTaggedUnion, OutInlineHeader,
					OutError))
			{
				return false;
			}
			
			OutStruct.DataMembers.Add({DataMemberName,  "F" + DataMemberType});
			OutInlineHeader.AddTaggedUnion(NewTaggedUnion);

			continue;
		}

		if (AlgebraicKind == SATS::EType::Ref)
		{
			const auto Index = DataMemberAlgebraicType->Ref.Index;
			const auto& Referenced = ExportedTypes[Index];

			FDataMember DataMember;
			DataMember.Name = FCommon::ToPascalCase(RawName);
			DataMember.Type = FCommon::MakeStructName(Referenced.Name.Name, ModuleName);
			DataMember.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(AlgebraicKind);
			DataMember.Comment = RawName + ": " + Referenced.Name.Name;
			
			OutStruct.DataMembers.Add(DataMember);
			
			continue;
		}

		if (SATS::IsBuiltinWithNativeRepresentation(AlgebraicKind))
		{
			FDataMember DataMember;
			DataMember.Name = FCommon::ToPascalCase(RawName);
			DataMember.Type = SATS::MapBuiltinToUnreal(SATS::TypeToString(AlgebraicKind), false);
			DataMember.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(AlgebraicKind);
			DataMember.Comment = RawName + ": " + SATS::TypeToString(AlgebraicKind);
			
			OutStruct.DataMembers.Add(DataMember);

			WarnTypes(AlgebraicKind);

			continue;
		}

		if (AlgebraicKind == SATS::EType::Invalid)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid SATS-JSON type found in Typespace codegen"));
		}

		UE_LOG(LogTemp, Error, TEXT("Internal inconsistency found in Typespace codegen - unhandled SATS-JSON type"));
		
	}
	
	return true;
}

bool FTypespaceStructIRBuilder::GenerateNewTaggedUnion(
	const FString& ModuleName,
	const TArray<SATS::FExportedType>& ExportedTypes,
	const FString& UnionName,
	const SATS::FSumType& Sum,
	FTaggedUnion& OutTaggedUnion,
	FHeader &OutInlineHeader,
	FString &OutError)
{
	// OutTaggedUnion.BaseName = UnionName.IsSet() ? UnionName.GetValue() : GenerateBaseNameForInlineTaggedUnion();
	OutTaggedUnion.Name = UnionName;

	const auto UnrealFormattedModuleName = FCommon::ToPascalCase(ModuleName);
	
	OutTaggedUnion.bIsReflected = true;
	OutTaggedUnion.SubCategory = UnrealFormattedModuleName;

	// const auto Anonymous = SATS::FOptionalString();
	
	UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating Tagged Union: F%s"), *OutTaggedUnion.Name);
	for (const auto& [VariantOptionalName, VariantAlgebraicType] : Sum.Options)
	{
		if (!VariantAlgebraicType.IsValid())
		{
			OutError = FString::Printf(TEXT("invalid pointer while generating tagged union (Algebraic Sum) F%s"), *OutTaggedUnion.Name);
			return false;
		}
		
		const auto RawName = GetDataMemberName(VariantOptionalName);
		const auto Tag = VariantAlgebraicType->Type;
		
		if (Tag == SATS::EType::Product)
		{
			const auto DataMemberName = FCommon::ToPascalCase(RawName);
			const auto DataMemberType = "F" + DataMemberName;
			
			const auto &ProductElement = VariantAlgebraicType->Product;
			FStruct NewStruct;
			if (!GenerateNewStruct(ModuleName, ExportedTypes, DataMemberType, ProductElement, NewStruct, OutInlineHeader, OutError))
			{
				return false;
			}
			
			OutTaggedUnion.Variants.Add({DataMemberName, DataMemberType});
			OutTaggedUnion.OptionTags.Add(NewStruct.Name);
			OutInlineHeader.AddStruct(NewStruct);

			continue;
		}

		if (Tag == SATS::EType::Sum)
		{
			const auto DataMemberName = FCommon::ToPascalCase(RawName);
			const auto DataMemberType = DataMemberName;
			
			const auto &SumElement = VariantAlgebraicType->Sum;
			FTaggedUnion NewTaggedUnion;
			if (!GenerateNewTaggedUnion(ModuleName, ExportedTypes, DataMemberType, SumElement, NewTaggedUnion, OutInlineHeader, OutError))
			{
				return false;
			}
			
			OutTaggedUnion.Variants.Add({DataMemberName, "F" + DataMemberType});
			OutTaggedUnion.OptionTags.Add(DataMemberType);
			OutInlineHeader.AddTaggedUnion(NewTaggedUnion);

			continue;
		}

		if (Tag == SATS::EType::Ref)
		{
			const auto Index = VariantAlgebraicType->Ref.Index;
			const auto& Referenced = ExportedTypes[Index];
			const FString Name = FCommon::ToPascalCase(RawName);
			const FString Type = Referenced.Name.Name;
			
			FDataMember Variant;
            Variant.Name = Name;
            Variant.Type = FCommon::MakeStructName(Referenced.Name.Name, ModuleName);
            Variant.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(Tag);
            Variant.Comment = RawName + ": " + Referenced.Name.Name;
            			
			OutTaggedUnion.Variants.Add(Variant);
			
			continue;
		}

		if (SATS::IsBuiltinWithNativeRepresentation(Tag))
		{
			FDataMember Variant;
            Variant.Name = FCommon::ToPascalCase(RawName);
            Variant.Type = SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag), false);
            Variant.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(Tag);
            Variant.Comment = RawName + ": " + SATS::TypeToString(Tag);
		
			OutTaggedUnion.Variants.Add(Variant);

			WarnTypes(Tag);

			continue;
		}

		if (Tag == SATS::EType::Invalid)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid SATS-JSON type found in Typespace codegen"));
		}

		UE_LOG(LogTemp, Error, TEXT("Internal inconsistency found in Typespace codegen - unhandled SATS-JSON type"));
		
	}
	
	return true;
}

bool FTypespaceStructIRBuilder::BuildTypesHeaders(
	const FString& ModuleName,
	const SATS::FTypespace& Typespace,
	const TArray<SATS::FExportedType>& ExportedTypes,
	FHeader &OutExported,
	FHeader &OutInline,
	FString &OutError)
{
	// TODO: check if 'ExportedTypes' have matched Refs in 'Typespace'
	
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

		if (AlgebraicType.Type != SATS::EType::Product)
		{
			OutError = FString::Printf(TEXT("Header generation for types in 'typespace' other than C++ structs "
				"(i.e. 'Product' Sats-Type) not implemented - problem occured with type '%i'"), Index);
			return false;
		}
		
		FStruct Struct;
		Struct.MetadataSpecifiers.Add("Category", "\"SpacetimeDB|" + UnrealFormattedModuleName + "\"");

		if (FString StructName = FCommon::MakeStructName(Type.Name.Name, ModuleName);
			!GenerateNewStruct(
				ModuleName,
				ExportedTypes, StructName,
				AlgebraicType.Product,
				Struct, OutInline, OutError))
		{
			return false;
		}
		
		OutExported.AddStruct(Struct);
	}
		
	return true;
}