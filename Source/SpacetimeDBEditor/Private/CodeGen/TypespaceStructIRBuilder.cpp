#include "TypespaceStructIRBuilder.h"

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

FString GenerateNameForAnonymousAttribute()
{
	static int32 Unnamed = 0;

	return "AnonymousField_" + FString::FromInt(Unnamed++);
}

FString GetAttributeName(const SATS::FOptionalString& Name)
{
	if (Name.IsSet()) return Name.GetValue();

	return GenerateNameForAnonymousAttribute();
}

void WarnTypes(const SATS::EType Tag)
{
	if (!SATS::IsReflectedInUnreal(Tag))
	{
		FString UEType = SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag), true);
		FString UETypeAlt = SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag), false);
		FString SpacetimeBuiltIn = SATS::TypeToString(Tag);

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
		UInt256.Attributes.Add({"Value", "FString"});
		UInt256.bIsReflected = true;
		UInt256.Specifiers.Add("BlueprintType");
		UInt256.MetadataSpecifiers.Add("Category", "\"SpacetimeDB\"");
		UInt256.Comment = TEXT("Provides SATS-JSON U256 support; Unreal UBT lacks uint256 reflection.");

		Header.AddStruct(UInt256);
	}

	{
		FStruct Int256;
		Int256.Name = TEXT("FInt256");
		Int256.Attributes.Add({"Value", "FString"});
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
		E.Name  = TU.BaseName;
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
		for (auto& Attr : S.Attributes)
			E.Depends.Add(Attr.Type);
		OutElements.Add(MoveTemp(E));
	}
}

bool FTypespaceStructIRBuilder::GenerateNewStruct(
	const FString& ModuleName,
	const TArray<SATS::FExportedType>& ExportedTypes,
	const SATS::FOptionalString& StructName,
	const SATS::FProductType& Product,
	FStruct& OutStruct,
	FHeader &OutInlineHeader,
	FString &OutError)
{	
	OutStruct.Name = StructName.IsSet() ? StructName.GetValue() : GenerateNameForInlineStruct();

	const auto UnrealFormattedModuleName = FCommon::ToPascalCase(ModuleName);
	
	OutStruct.bIsReflected = true;
	OutStruct.Specifiers.Add("BlueprintType");
	OutStruct.MetadataSpecifiers.Add("Category", "\"SpacetimeDB|" + UnrealFormattedModuleName + "\"");

	UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating Struct: %s"), *OutStruct.Name);
	for (const auto& [AttributeOptionalName, AttributeAlgebraicType] : Product.Elements)
	{
		if (!AttributeAlgebraicType.IsValid())
		{
			OutError = FString::Printf(TEXT("invalid pointer while generating struct %s"), *OutStruct.Name);
			return false;
		}
		
		const auto RawName = GetAttributeName(AttributeOptionalName);
		const auto Tag = AttributeAlgebraicType->Tag;
		
		if (Tag == SATS::EType::Product)
		{
			const auto Anonymous = SATS::FOptionalString();
			const auto &ProductElement = AttributeAlgebraicType->Product;
			FStruct NewStruct;
			if (!GenerateNewStruct(ModuleName, ExportedTypes, Anonymous, ProductElement, NewStruct, OutInlineHeader, OutError))
			{
				return false;
			}

			const auto NewStructName = FCommon::ToPascalCase(RawName);
			OutStruct.Attributes.Add({
				NewStructName,
				NewStruct.Name});
			OutInlineHeader.AddStruct(NewStruct);

			continue;
		}

		if (Tag == SATS::EType::Sum)
		{
			const auto Anonymous = SATS::FOptionalString();
			const auto &SumElement = AttributeAlgebraicType->Sum;
			FTaggedUnion NewTaggedUnion;
			if (!GenerateNewTaggedUnion(
					ModuleName, ExportedTypes,
					Anonymous,  SumElement,
					NewTaggedUnion, OutInlineHeader,
					OutError))
			{
				return false;
			}

			const auto NewTaggedUnionName = FCommon::ToPascalCase(RawName);
			
			OutStruct.Attributes.Add({NewTaggedUnionName,  "F" + NewTaggedUnion.BaseName});
			OutInlineHeader.AddTaggedUnion(NewTaggedUnion);

			continue;
		}

		if (Tag == SATS::EType::Ref)
		{
			const auto Index = AttributeAlgebraicType->Ref.Index;
			const auto& Referenced = ExportedTypes[Index];

			FAttribute Attribute;
			Attribute.Name = FCommon::ToPascalCase(RawName);
			Attribute.Type = FCommon::MakeStructName(Referenced.Name.Name, ModuleName);
			Attribute.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(Tag);
			Attribute.Comment = RawName + ": " + Referenced.Name.Name;
			
			OutStruct.Attributes.Add(Attribute);
			
			continue;
		}

		if (SATS::IsBuiltinWithNativeRepresentation(Tag))
		{
			FAttribute Attribute;
			Attribute.Name = FCommon::ToPascalCase(RawName);
			Attribute.Type = SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag), false);
			Attribute.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(Tag);
			Attribute.Comment = RawName + ": " + SATS::TypeToString(Tag);
			
			OutStruct.Attributes.Add(Attribute);

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

bool FTypespaceStructIRBuilder::GenerateNewTaggedUnion(
	const FString& ModuleName,
	const TArray<SATS::FExportedType>& ExportedTypes,
	const SATS::FOptionalString& UnionName,
	const SATS::FSumType& Sum,
	FTaggedUnion& OutTaggedUnion,
	FHeader &OutInlineHeader,
	FString &OutError)
{
	OutTaggedUnion.BaseName = UnionName.IsSet() ? UnionName.GetValue() : GenerateBaseNameForInlineTaggedUnion();

	const auto UnrealFormattedModuleName = FCommon::ToPascalCase(ModuleName);
	
	OutTaggedUnion.bIsReflected = true;
	OutTaggedUnion.SubCategory = UnrealFormattedModuleName;

	UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating Tagged Union: F%s"), *OutTaggedUnion.BaseName);
	for (const auto& [VariantOptionalName, VariantAlgebraicType] : Sum.Options)
	{
		if (!VariantAlgebraicType.IsValid())
		{
			OutError = FString::Printf(TEXT("invalid pointer while generating tagged union (Algebraic Sum) F%s"), *OutTaggedUnion.BaseName);
			return false;
		}
		
		const auto RawName = GetAttributeName(VariantOptionalName);
		const auto Tag = VariantAlgebraicType->Tag;
		
		if (Tag == SATS::EType::Product)
		{
			const auto Anonymous = SATS::FOptionalString();
			const auto &ProductElement = VariantAlgebraicType->Product;
			FStruct NewStruct;
			if (!GenerateNewStruct(ModuleName, ExportedTypes, Anonymous, ProductElement, NewStruct, OutInlineHeader, OutError))
			{
				return false;
			}

			const auto NewStructName = FCommon::ToPascalCase(RawName);
			OutTaggedUnion.Variants.Add({NewStructName, NewStruct.Name});
			OutTaggedUnion.OptionTags.Add(NewStruct.Name);
			OutInlineHeader.AddStruct(NewStruct);

			continue;
		}

		if (Tag == SATS::EType::Sum)
		{
			const auto Anonymous = SATS::FOptionalString();
			const auto &SumElement = VariantAlgebraicType->Sum;
			FTaggedUnion NewTaggedUnion;
			if (!GenerateNewTaggedUnion(ModuleName, ExportedTypes, Anonymous, SumElement, NewTaggedUnion, OutInlineHeader, OutError))
			{
				return false;
			}

			const auto NewTaggedUnionName = FCommon::ToPascalCase(RawName);
			OutTaggedUnion.Variants.Add({NewTaggedUnion.BaseName, "F" + NewTaggedUnionName});
			OutTaggedUnion.OptionTags.Add(NewTaggedUnion.BaseName);
			OutInlineHeader.AddTaggedUnion(NewTaggedUnion);

			continue;
		}

		if (Tag == SATS::EType::Ref)
		{
			const auto Index = VariantAlgebraicType->Ref.Index;
			const auto& Referenced = ExportedTypes[Index];
			const FString Name = FCommon::ToPascalCase(RawName);
			const FString Type = Referenced.Name.Name;
			
			FAttribute Variant;
            Variant.Name = Name;
            Variant.Type = FCommon::MakeStructName(Referenced.Name.Name, ModuleName);
            Variant.DefaultValue = FSpacetimeConfig::GetDefaultValueForType(Tag);
            Variant.Comment = RawName + ": " + Referenced.Name.Name;
            			
			OutTaggedUnion.Variants.Add(Variant);
			
			continue;
		}

		if (SATS::IsBuiltinWithNativeRepresentation(Tag))
		{
			FAttribute Variant;
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

	OutInline.Includes.Add({"CoreMinimal.h", true});
	OutInline.Includes.Add({InlineTypesHeaderName + ".generated.h", true});
	
	OutExported.Includes.Add({"CoreMinimal.h", true});
	OutExported.Includes.Add({InlineTypesHeaderName + ".h", true});
	OutExported.Includes.Add({ExportedTypesHeaderName + ".generated.h", true});

	AddMissingBuiltIns(OutInline);

	for (const auto& Type : ExportedTypes)
	{
		const auto Index = Type.TypeRef;
		const auto &AlgebraicType = Typespace.TypeEntries[Index];

		if (AlgebraicType.Tag != SATS::EType::Product)
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

bool FTypespaceStructIRBuilder::BuildTypespaceHeader_Deprecated(
	const FString& ModuleName,
	const FString& HeaderBaseName,
	const SATS::FTypespace& Typespace,
	const TArray<SATS::FExportedType>& ExportedTypes,
	FHeader& OutHeader,
	FString& OutError)
{
	TArray<SATS::FExportedType> Types = ExportedTypes;
	Algo::Sort(Types, [](const SATS::FExportedType& EntryA, const  SATS::FExportedType& EntryB)
	{
		return EntryA.TypeRef < EntryB.TypeRef;
	});
	
	const FString UnrealFormattedModuleName = FCommon::ToPascalCase(ModuleName);
	
	if (Typespace.TypeEntries.Num() != Types.Num())
	{
		OutError = FString::Printf(
			TEXT("Inconsistent number of entries in "
			"'typespace' (%i entries) and "
			"'types' (%i entries) "
			"in RawModuleDef"),
			Typespace.TypeEntries.Num(),
			Types.Num());
		return false;
	}
	// TODO: also check if types <-> typespace (if they have 1:1 matching)
	
	OutHeader.Includes.Add({"CoreMinimal.h", true});
	OutHeader.Includes.Add({HeaderBaseName + ".generated.h", true});

	AddMissingBuiltIns(OutHeader);

	for (const auto& Type : Types)
	{
		const auto Index = Type.TypeRef;
		const auto &AlgebraicType = Typespace.TypeEntries[Index];

		if (AlgebraicType.Tag != SATS::EType::Product)
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
				Types, StructName,
				AlgebraicType.Product,
				Struct, OutHeader, OutError))
		{
			return false;
		}
		
		OutHeader.AddStruct(Struct);
	}
	
	return true;
}