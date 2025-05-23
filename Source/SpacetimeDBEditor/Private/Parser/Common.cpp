#include "Common.h"

FString FCommon::ArrayToString(const TArray<FString>& StringArray)
{
	FString Result;
	for (const auto& String : StringArray)
	{
		Result += String + TEXT(", ");
	}
	return "[" + Result.LeftChop(2) + "]";
}

FString FCommon::CreateUniqueName()
{
	static int32 UniqueNameCounter = 0;
	return FString::Printf(TEXT("Unnamed%d"), UniqueNameCounter++);
}

SATS::FOptionalString FCommon::GetOptionalString(const TSharedPtr<FJsonObject>& OptionalString)
{
	SATS::FOptionalString ReturnValue;
	if ( OptionalString->HasField(TEXT("some")) )
	{
		ReturnValue = OptionalString->GetStringField(TEXT("some"));
	}
	else if (!OptionalString->HasField(TEXT("none")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Unexpected JsonObject for optional String field"));
	}
        
	return ReturnValue;
}

FString FCommon::UnwrapOptionString(const TSharedPtr<FJsonObject>& OptionObj)
{
	const FString SomeKey = TEXT("some");
	return OptionObj->HasField(SomeKey)
		? OptionObj->GetStringField(SomeKey)
		: FString();
}

FString FCommon::ToPascalCase(const FString& InString)
{
    FString Result;
    bool bCapNext = true;

    const int32 Len = InString.Len();
    for (int32 i = 0; i < Len; ++i)
    {
        const TCHAR Ch = InString[i];

        if (FChar::IsAlnum(Ch))
        {
            // If we see an uppercase character in the middle of a word that follows a lowercase one,
            // treat it as the start of a new PascalCase word.
            if (!bCapNext 
                && FChar::IsUpper(Ch) 
                && i > 0 
                && FChar::IsLower(InString[i - 1]))
            {
                bCapNext = true;
            }

            if (bCapNext)
            {
                Result += FChar::ToUpper(Ch);
                bCapNext = false;
            }
            else
            {
                Result += FChar::ToLower(Ch);
            }
        }
        else
        {
            // Any non‐alphanumeric (underscore, dash, space, etc.) triggers a new word
            bCapNext = true;
        }
    }

    return Result;
};

bool FCommon::ParseRequiredArray(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field,
						TArray<TSharedPtr<FJsonValue>>& OutArray, FString& OutError)
{
	if (!Parent->HasTypedField<EJson::Array>(Field))
	{
		OutError = FString::Printf(TEXT("Missing or invalid required array '%s'"), Field);
		return false;
	}
	OutArray = Parent->GetArrayField(Field);
	return true;
}

// Reads a required JSON object field, returning nullptr + error if missing/not-object
TSharedPtr<FJsonObject> FCommon::ParseRequiredObject(
	const TSharedPtr<FJsonObject>& Parent,
	const FString& Field,
	FString& OutError)
{
	if (!Parent->HasField(Field))
	{
		OutError = FString::Printf(TEXT("Missing required JSON object '%s'"), *Field);
		return nullptr;
	}
	TSharedPtr<FJsonObject> Obj = Parent->GetObjectField(Field);
	if (!Obj.IsValid())
	{
		OutError = FString::Printf(TEXT("Field '%s' is not a JSON object"), *Field);
		return nullptr;
	}
	return Obj;
}

// Reads a required JSON string field, returning nullptr + error if missing/not-object
bool FCommon::ParseRequiredString(const TSharedPtr<FJsonObject>& Parent,
							const FString& Field,
							FString &OutString,
							FString& OutError)
{
	if (!Parent->HasField(Field))
	{
		OutError = FString::Printf(TEXT("Missing field '%s'"), *Field);
		return false;
	}
	OutString = Parent->GetStringField(Field);
        
	return true;
}

bool FCommon::ParseNameAndAlgebraicType(
	const TSharedPtr<FJsonObject>& NameAndAlgTypePair,
	SATS::FOptionalString &OptionalName,
	SATS::FAlgebraicType& AlgebraicOut,
	FString& OutError)
{
	if (!NameAndAlgTypePair.IsValid() || NameAndAlgTypePair->Values.Num() != 2)
	{
		OutError = TEXT("Expected JSON object with two fields: 'name' and 'algebraic_type'");
		return false;
	}
    
	// ---------------------------------
	// Parse field 'name'
	const TSharedPtr<FJsonObject>& NameObject = NameAndAlgTypePair->GetObjectField(TEXT("name"));
	if (!NameObject.IsValid())
	{
		OutError = TEXT("Invalid entry in optional field 'name', expected SATS-JSON object");
		return false;
	}
	OptionalName = GetOptionalString(NameObject);

	// ---------------------------------
	// Parse field 'algebraic_type'
	if (!NameAndAlgTypePair->HasTypedField<EJson::Object>(TEXT("algebraic_type")))
	{
		OutError = TEXT("Missing 'algebraic_type' key in SATS-JSON object");
		if (OptionalName.IsSet())
		{
			OutError += FString::Printf(TEXT(", found name '%s'"), *OptionalName.GetValue());
		}
		return false;
	}
	const TSharedPtr<FJsonObject>& AlgebraicTypeObject = NameAndAlgTypePair->GetObjectField(TEXT("algebraic_type"));

	if (!AlgebraicTypeObject.IsValid())
	{
		auto Name = OptionalName.IsSet() ? OptionalName.GetValue() : TEXT("unnamed");
		OutError = FString::Printf(
			TEXT("Either not present or invalid 'algebraic_type' in TypeDef element (%s)"), *Name);
		return false;
	}

	if (AlgebraicTypeObject->Values.Num() != 1)
	{
		OutError = TEXT("Invalid 'algebraic_type' in TypeDef element");
		return false;
	}
	
	if (!ResolveAlgebraicType(AlgebraicTypeObject, AlgebraicOut, OutError))
	{
		auto OutErrorTemp = FString::Printf(TEXT("Failed to resolve Algebraic Type"));
		if (OptionalName.IsSet())
		{
			OutErrorTemp += FString::Printf(TEXT(" of named SATS-JSON object '%s'"), *OptionalName.GetValue());
		}
		else
		{
			OutErrorTemp += TEXT(" of unnamed SATS-JSON object");
		}
		OutError = OutErrorTemp + ": " + OutError; 
		return false;
	}

	return true;
}

bool FCommon::ValidateAndGetSatsKindInTypespaceEntry(const TSharedPtr<FJsonObject>& TypeObj,
		SATS::EType &SatsKind, FString& OutError)
{        
	TArray<FString> Keys;
	TypeObj->Values.GetKeys(Keys);
	if (Keys.Num() != 1)
	{
		OutError = TEXT("Invalid entry in 'types' array, expected single key 'elements' for type kind");
		return false;
	}

	SatsKind = SATS::StringToType(Keys[0]);

	if (SatsKind == SATS::EType::Invalid)
	{
		OutError = FString::Printf(
			TEXT("Invalid entry in 'types' array, expected 'Product' or 'Sum', found '%s'"), *Keys[0]);
		return false;
	}

	return true;
}

bool FCommon::ParseSum(const TArray<TSharedPtr<FJsonValue>>& Variants,
        SATS::FSumType& SumOut, FString& OutError)
{
    // Clear any existing options
    SumOut.Options.Empty();
    
    for (const auto& Option : Variants)
    {
        const TSharedPtr<FJsonObject> OptionObj = Option->AsObject();
        if (!OptionObj.IsValid())
        {
            OutError = TEXT("Invalid entry in 'types' array, expected JSON objects");
            return false;
        }

        TSharedPtr<SATS::FAlgebraicType> AlgebraicType = MakeShared<SATS::FAlgebraicType>();
        TOptional<FString> NameString;
        if (!ParseNameAndAlgebraicType(OptionObj, NameString, *AlgebraicType, OutError))
        {
            return false;
        }
        
        SumOut.Options.Add({NameString, MoveTemp(AlgebraicType)});
    }
    
    return true;
}

bool FCommon::ParseBuiltin(const TSharedPtr<FJsonObject>& BuiltinObj,
    SATS::FBuiltinType &BuiltinOut, FString& OutError)
{
    TArray<FString> Keys;
    BuiltinObj->Values.GetKeys(Keys);

    if (Keys.Num() != 1)
    {
        OutError = TEXT("Invalid entry in 'types' array, expected single key for type kind");
        return false;
    }

    const FString BuiltinTypeStr = Keys[0];
    BuiltinOut.Tag = SATS::StringToBuiltinType(BuiltinTypeStr);
    if (BuiltinOut.Tag == SATS::EBuiltinType::Array || BuiltinOut.Tag == SATS::EBuiltinType::Map)
    {
        OutError = FString::Printf(TEXT("SATS parsing of builtin types Array|Map not implemented"));
        return false;
    }

    return true;
}


bool FCommon::ResolveAlgebraicType(
	const TSharedPtr<FJsonObject>& SatsJsonObject,
    SATS::FAlgebraicType& AlgebraicOut,
    FString& OutError)
{
    SATS::EType SatsKind;
    if (!ValidateAndGetSatsKindInTypespaceEntry(SatsJsonObject, SatsKind, OutError))
    {
    	OutError = TEXT("While resolving Algebraic Type of a SATS-JSON TypeDef: ") + OutError;
        return false;
    }
    
    if (SatsKind == SATS::EType::Product)
    {
    	auto ProductObject = SatsJsonObject->GetObjectField(TEXT("Product"));
    	if (!ProductObject.IsValid())
    	{
    		OutError = TEXT("Internal inconsistency in parsing SATS-JSON Product Algebraic Type. "
					  "Expected validated 'Product' object field, found none");
    		return false;
    	}
    	
        const TArray<TSharedPtr<FJsonValue>> *Elements;
        if (!ProductObject->TryGetArrayField(TEXT("elements"), Elements))
        {
        	
            OutError = TEXT("Invalid entry in SATS-JSON Product Algebraic Type. Expected 'elements' array field");
            return false;
        }
        SATS::FProductType Product;
        if (!ParseProduct(*Elements, Product, OutError))
        {
            return false;
        }

        AlgebraicOut.Tag = SATS::EType::Product;
        AlgebraicOut.Product = MoveTemp(Product);
        return true;                    
    }

    if (SatsKind == SATS::EType::Sum)
    {
    	auto SumObject = SatsJsonObject->GetObjectField(TEXT("Sum"));
    	if (!SumObject.IsValid())
    	{
    		OutError = TEXT("Internal inconsistency in parsing SATS-JSON Sum Algebraic Type. "
					  "Expected validated 'Sum' object field, found none");
    		return false;
    	}
    	
    	
        const TArray<TSharedPtr<FJsonValue>> *Variants;
        if (!SumObject->TryGetArrayField(TEXT("variants"), Variants))
        {
            OutError = TEXT("Invalid entry in SATS-JSON Sum Algebraic Type. Expected 'variants' array field");
            return false;
        }
        
        SATS::FSumType Sum;
        if (!ParseSum(*Variants, Sum, OutError))
        {
            return false;
        }
        
        AlgebraicOut.Sum = MoveTemp(Sum);
        AlgebraicOut.Tag = SATS::EType::Sum;
        return true;
    }

    if (SatsKind == SATS::EType::Ref)
    {
        OutError = TEXT("Parsing of Ref SATS-JSON types not implemented");
        return false;
    }

    // else it is SATS Builtin 
    // "if SatsKind \in {SATS Builtin}
    {
        const TSharedPtr<FJsonObject>& BuiltinObj = SatsJsonObject;
        SATS::FBuiltinType Builtin;
        if (!ParseBuiltin(BuiltinObj, Builtin, OutError))
        {
        	OutError = TEXT("While resolving Algebraic Type of a SATS-JSON BuiltIn: ") + OutError;
            return false;
        }

        // We can safely cast because the enums are correctly mapped
        AlgebraicOut.Tag = static_cast<SATS::EType>(Builtin.Tag);
        AlgebraicOut.Builtin = MoveTemp(Builtin);
        return true;
    }
}

bool FCommon::ParseProduct(const TArray<TSharedPtr<FJsonValue>>& Elements,
    SATS::FProductType& ProductOut, FString& OutError)
{
	
    for (const auto& Element : Elements)
    {
        const TSharedPtr<FJsonObject> ElementObj = Element->AsObject();
    
        if (!ElementObj.IsValid())
        {
            OutError = TEXT("Invalid element in ProductType");
            return false;
        }

        TSharedPtr<SATS::FAlgebraicType> AlgebraicType = MakeShared<SATS::FAlgebraicType>();
        SATS::FOptionalString NameString;
        if (!ParseNameAndAlgebraicType(ElementObj, NameString, *AlgebraicType, OutError))
        {
        	int ElementIdx = ProductOut.Elements.Num();
        	OutError = FString::Printf(
        		TEXT("While parsing 'name' and 'algebraic_type' fields of Product's element %i: "), ElementIdx) + OutError;
	        return false;
        }
    	
        ProductOut.Elements.Add({NameString, MoveTemp(AlgebraicType)});
    }
    
    return true;
}

