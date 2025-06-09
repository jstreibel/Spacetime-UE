#include "TypespaceCodegen.h"

FString IntToString(const uint32 Value)
{
	return FString::Printf(TEXT("%02d"), Value);
}

FString GenerateNameForInlineStruct()
{
	static int32 Unnamed = 0;

	return "InlineStruct_" + IntToString(Unnamed++);
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

FStruct GenerateNewStruct(const SATS::FOptionalString& StructName, const SATS::FProductType& Product, FHeader &OutHeader)
{
	FStruct OutStruct;
	OutStruct.Name = StructName.IsSet() ? StructName.GetValue() : GenerateNameForInlineStruct();
	OutStruct.bIsReflected = true;
	OutStruct.Specifiers.Add("BlueprintType");

	for (const auto& [AttributeOptionalName, AttributeAlgebraicType] : Product.Elements)
	{
		const auto AttributeName = GetAttributeName(AttributeOptionalName);
		if (AttributeAlgebraicType->Tag == SATS::EType::Product)
		{
			auto NewStruct = GenerateNewStruct(GenerateNameForInlineStruct(), AttributeAlgebraicType->Product, OutHeader);
			OutStruct.Attributes.Add(NewStruct.Name, AttributeName);

			OutHeader.Structs.Add(NewStruct);
			
			
		}
	}
	
	return OutStruct;
}

bool FTypespaceCodegen::FromIntermediateRepresentation(
	const FString &HeaderBaseName,
	const SATS::FTypespace& Typespace,
	FHeader &OutHeader,
	FString &OutError)
{
	OutHeader.Includes.Add({"CoreMinimal.h", true});
	OutHeader.Includes.Add({"UObject/NoExportTypes.h", true});
	OutHeader.Includes.Add({HeaderBaseName + ".generated.h"});

	int TypespaceRef = 0;
	for (const auto& AlgebraicType : Typespace.TypeEntries)
	{
		if (AlgebraicType.Tag != SATS::EType::Product)
		{
			OutError = FString::Printf(TEXT("Header generation for types other than C++ structs (i.e. 'Product' Sats-Type)"
							  " not implemented - problem occured with type '%i'"), TypespaceRef);
			return false;
		}

		const FString Name = "ProductType" + IntToString(TypespaceRef);
		FStruct Struct = GenerateNewStruct(Name,  AlgebraicType.Product);

		TypespaceRef++;
	}
	
	return true;
}
