#include "TypespaceStructGen.h"

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

		Header.Structs.Add(UInt256);
	}

	{
		FStruct Int256;
		Int256.Name = TEXT("FInt256");
		Int256.Attributes.Add({"Value", "FString"});
		Int256.bIsReflected = true;
		Int256.Specifiers.Add("BlueprintType");
		Int256.MetadataSpecifiers.Add("Category", "\"SpacetimeDB\"");
		Int256.Comment = TEXT("Provides SATS-JSON I256 support; Unreal UBT lacks int256 reflection.");
		
		Header.Structs.Add(Int256);
	}
}

bool GenerateNewTaggedUnion(const FString& ModuleName, const TArray<SATS::FExportedType>& ExportedTypes,
	const SATS::FOptionalString& UnionName,	const SATS::FSumType& Sum, FTaggedUnion& OutTaggedUnion,
	FHeader &OutHeader, FString &OutError);

bool GenerateNewStruct(
	const FString& ModuleName,
	const TArray<SATS::FExportedType>& ExportedTypes,
	const SATS::FOptionalString& StructName,
	const SATS::FProductType& Product,
	FStruct& OutStruct,
	FHeader &OutHeader,
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
			if (!GenerateNewStruct(ModuleName, ExportedTypes, Anonymous, ProductElement, NewStruct, OutHeader, OutError))
			{
				return false;
			}

			const auto NewStructName = FCommon::ToPascalCase(RawName);
			OutStruct.Attributes.Add({
				NewStructName,
				NewStruct.Name});
			OutHeader.Structs.Add(NewStruct);

			continue;
		}

		if (Tag == SATS::EType::Sum)
		{
			const auto Anonymous = SATS::FOptionalString();
			const auto &SumElement = AttributeAlgebraicType->Sum;
			FTaggedUnion NewTaggedUnion;
			if (!GenerateNewTaggedUnion(ModuleName, ExportedTypes, Anonymous, SumElement, NewTaggedUnion, OutHeader, OutError))
			{
				return false;
			}

			const auto NewTaggedUnionName = FCommon::ToPascalCase(RawName);
			OutStruct.Attributes.Add({NewTaggedUnionName, NewTaggedUnion.BaseName});
			OutHeader.TaggedUnions.Add(NewTaggedUnion);

			continue;
		}

		if (Tag == SATS::EType::Ref)
		{
			const auto Index = AttributeAlgebraicType->Ref.Index;
			const auto& Referenced = ExportedTypes[Index];
			const FString Name = FCommon::ToPascalCase(RawName);
			const FString Type = Referenced.Name.Name;
			
			OutStruct.Attributes.Add({
				Name,
				FCommon::MakeStructName(Referenced.Name.Name, ModuleName),
				RawName + ": " + Referenced.Name.Name});
			
			continue;
		}

		if (SATS::IsBuiltinWithNativeRepresentation(Tag))
		{
			const FString Name = FCommon::ToPascalCase(RawName);
			OutStruct.Attributes.Add({
				Name,
				SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag), false),
			 RawName + ": " + SATS::TypeToString(Tag) });

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

bool GenerateNewTaggedUnion(
	const FString& ModuleName,
	const TArray<SATS::FExportedType>& ExportedTypes,
	const SATS::FOptionalString& UnionName,
	const SATS::FSumType& Sum,
	FTaggedUnion& OutTaggedUnion,
	FHeader &OutHeader,
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
			if (!GenerateNewStruct(ModuleName, ExportedTypes, Anonymous, ProductElement, NewStruct, OutHeader, OutError))
			{
				return false;
			}

			const auto NewStructName = FCommon::ToPascalCase(RawName);
			OutTaggedUnion.Variants.Add({NewStructName, NewStruct.Name});
			OutTaggedUnion.OptionTags.Add(NewStruct.Name);
			OutHeader.Structs.Add(NewStruct);

			continue;
		}

		if (Tag == SATS::EType::Sum)
		{
			const auto Anonymous = SATS::FOptionalString();
			const auto &SumElement = VariantAlgebraicType->Sum;
			FTaggedUnion NewTaggedUnion;
			if (!GenerateNewTaggedUnion(ModuleName, ExportedTypes, Anonymous, SumElement, NewTaggedUnion, OutHeader, OutError))
			{
				return false;
			}

			const auto NewTaggedUnionName = FCommon::ToPascalCase(RawName);
			OutTaggedUnion.Variants.Add({NewTaggedUnionName, NewTaggedUnion.BaseName});
			OutTaggedUnion.OptionTags.Add(NewTaggedUnion.BaseName);
			OutHeader.TaggedUnions.Add(NewTaggedUnion);

			continue;
		}

		if (Tag == SATS::EType::Ref)
		{
			const auto Index = VariantAlgebraicType->Ref.Index;
			const auto& Referenced = ExportedTypes[Index];
			const FString Name = FCommon::ToPascalCase(RawName);
			const FString Type = Referenced.Name.Name;
			
			OutTaggedUnion.Variants.Add({
				Name,
				FCommon::MakeStructName(Referenced.Name.Name, ModuleName),
				RawName + ": " + Referenced.Name.Name});
			
			continue;
		}

		if (SATS::IsBuiltinWithNativeRepresentation(Tag))
		{
			const FString Name = FCommon::ToPascalCase(RawName);
			OutTaggedUnion.Variants.Add({
				Name,
				SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag), false),
			 RawName + ": " + SATS::TypeToString(Tag) });

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


bool FTypespaceStructGen::BuildHeaderLayoutFromIntermediateRepresentation(
	const FString& ModuleName,
	const FString& HeaderBaseName,
	const SATS::FTypespace& Typespace,
	const TArray<SATS::FExportedType>& ExportedTypes,
	FHeader &OutHeader,
	FString &OutError)
{
	TArray<SATS::FExportedType> SortedTypes = ExportedTypes;
	Algo::Sort(SortedTypes, [](const SATS::FExportedType& EntryA, const  SATS::FExportedType& EntryB)
	{
		return EntryA.TypeRef < EntryB.TypeRef;
	});
	
	const FString UnrealFormattedModuleName = FCommon::ToPascalCase(ModuleName);
	
	if (Typespace.TypeEntries.Num() != SortedTypes.Num())
	{
		OutError = FString::Printf(
			TEXT("Inconsistent number of entries in "
			"'typespace' (%i entries) and "
			"'types' (%i entries) "
			"in RawModuleDef"),
			Typespace.TypeEntries.Num(),
			SortedTypes.Num());
		return false;
	}
	// TODO: also check if types <-> typespace (if they have 1:1 matching)
	
	OutHeader.Includes.Add({"CoreMinimal.h", true});
	OutHeader.Includes.Add({HeaderBaseName + ".generated.h", true});

	AddMissingBuiltIns(OutHeader);

	for (const auto& Type : SortedTypes)
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
				SortedTypes, StructName,
				AlgebraicType.Product,
				Struct, OutHeader, OutError))
		{
			return false;
		}
		
		OutHeader.Structs.Add(Struct);
	}
	
	return true;
}
