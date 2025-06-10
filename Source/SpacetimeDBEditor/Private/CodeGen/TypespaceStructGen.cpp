#include "TypespaceStructGen.h"

#include "Parser/Common.h"

FString IntToString(const uint32 Value)
{
	return FString::Printf(TEXT("%02d"), Value);
}

FString GenerateNameForInlineStruct()
{
	static int32 Unnamed = 0;

	return "FInlineStruct_" + IntToString(Unnamed++);
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

bool GenerateNewStruct(
	const FString& ModuleName,
	const TArray<SATS::FExportedType>& Types,
	const SATS::FOptionalString& StructName,
	const SATS::FProductType& Product,
	FStruct& OutStruct,
	FHeader &OutHeader,
	FString &OutError)
{
	OutStruct.Name = StructName.IsSet() ? StructName.GetValue() : GenerateNameForInlineStruct();
	
	OutStruct.bIsReflected = true;
	OutStruct.Specifiers.Add("BlueprintType");
	OutStruct.MetadataSpecifiers.Add("Category", "\"SpacetimeDB\"");

	UE_LOG(LogTemp, Display, TEXT("[spacetime] Generating Struct: %s"), *OutStruct.Name);
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
			if (!GenerateNewStruct(ModuleName, Types, Anonymous, ProductElement, NewStruct, OutHeader, OutError))
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
			UE_LOG(LogTemp, Error, TEXT("SATS-JSON BuiltIn Sums are not currently implemented in Unreal codegen"));
			continue;
		}

		if (Tag == SATS::EType::Ref)
		{
			const auto Index = AttributeAlgebraicType->Ref.Index;
			const auto& Referenced = Types[Index];
			const FString Name = FCommon::ToPascalCase(RawName);
			const FString Type = Referenced.Name.Name;
			
			OutStruct.Attributes.Add({
				Name + " /* " + Referenced.Name.Name + " */",
				FCommon::MakeStructName(Referenced.Name.Name, ModuleName)});
			
			continue;
		}

		if (SATS::IsBuiltinWithNativeRepresentation(Tag))
		{
			const FString Name = FCommon::ToPascalCase(RawName);
			OutStruct.Attributes.Add({
				Name,
				SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag)),
			 RawName + ": " + SATS::TypeToString(Tag) });

			if (Tag == SATS::EType::U256)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("mapping a U256 to int256 since Unreal currently has no uint256 native type"));
			}
			
			if (Tag == SATS::EType::Array || Tag == SATS::EType::Map)
			{
				UE_LOG(LogTemp, Warning, TEXT("SATS-JSON BuiltIn types 'Array' and 'Map' "
					"not currently implemented in Unreal codegen"))
			}

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


bool FTypespaceStructGen::BuildHeaderLayoutFromIntermediateRepresentation(
	const FString& ModuleName,
	const FString& HeaderBaseName,
	const SATS::FTypespace& Typespace,
	const TArray<SATS::FExportedType>& Types,
	FHeader &OutHeader,
	FString &OutError)
{	
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
		Struct.MetadataSpecifiers.Add("Category", "\"SpacetimeDB\"");

		if (FString StructName = FCommon::MakeStructName(Type.Name.Name, ModuleName);
			!GenerateNewStruct(
				ModuleName,
				Types, StructName,
				AlgebraicType.Product,
				Struct, OutHeader, OutError))
		{
			return false;
		}
		
		OutHeader.Structs.Add(Struct);
	}
	
	return true;
}
