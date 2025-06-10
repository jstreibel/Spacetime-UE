#include "TypespaceCodegen.h"

#include "Animation/AnimAttributes.h"
#include "Parser/Common.h"
#include "UObject/PackageTrailer.h"

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
	const TArray<SATS::FExportedType>& Types,
	const SATS::FOptionalString& StructName,
	const SATS::FProductType& Product,
	const TSharedPtr<FStruct>& OutStruct,
	FHeader &OutHeader,
	FString &OutError)
{
	OutStruct->Name = StructName.IsSet() ? StructName.GetValue() : GenerateNameForInlineStruct();
	
	if (!OutStruct.IsValid())
	{
		OutError = FString::Printf(TEXT("invalid OutStruct pointer while generating new struct '%s'"), *OutStruct->Name);
		return false;
	}
	
	OutStruct->bIsReflected = true;
	OutStruct->Specifiers.Add("BlueprintType");

	UE_LOG(LogTemp, Display, TEXT("[spacetime] Generating Struct: %s"), *OutStruct->Name);
	for (const auto& [AttributeOptionalName, AttributeAlgebraicType] : Product.Elements)
	{
		if (!AttributeAlgebraicType.IsValid())
		{
			OutError = FString::Printf(TEXT("invalid pointer while generating struct %s"), *OutStruct->Name);
			return false;
		}
		
		const auto AttributeName = GetAttributeName(AttributeOptionalName);
		const auto Tag = AttributeAlgebraicType->Tag;
		
		if (Tag == SATS::EType::Product)
		{
			const auto Anonymous = SATS::FOptionalString();
			const auto &ProductElement = AttributeAlgebraicType->Product;
			TSharedPtr<FStruct> NewStruct = MakeShared<FStruct>();
			if (!GenerateNewStruct(Types, Anonymous, ProductElement, NewStruct, OutHeader, OutError))
			{
				return false;
			}
			
			OutStruct->Attributes.Add(NewStruct->Name, AttributeName);
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
			 
			OutStruct->Attributes.Add(Referenced.Name.Name, "F" + Referenced.Name.Name);
			continue;
		}

		if (SATS::IsBuiltinWithNativeRepresentation(Tag))
		{
			OutStruct->Attributes.Add(AttributeName, SATS::MapBuiltinToUnreal(SATS::TypeToString(Tag)));

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

bool FTypespaceCodegen::BuildHeaderLayoutFromIntermediateRepresentation(
	const FString &HeaderBaseName,
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
	OutHeader.Includes.Add({"UObject/NoExportTypes.h", true});
	OutHeader.Includes.Add({HeaderBaseName + ".generated.h"});

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
		
		TSharedPtr<FStruct> Struct = MakeShared<FStruct>();
		if (!GenerateNewStruct(Types, "F" + Type.Name.Name, AlgebraicType.Product,
			Struct, OutHeader, OutError))
		{
			return false;
		}

		OutHeader.Structs.Add(Struct);
	}
	
	return true;
}
