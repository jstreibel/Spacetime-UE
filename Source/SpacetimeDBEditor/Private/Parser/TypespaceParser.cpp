#include "TypespaceParser.h"


namespace 
{
    bool ValidateAndGetSatsKindInTypespaceEntry(const TSharedPtr<FJsonObject>& TypeObj,
        SATS::EKind &SatsKind, FString& OutError)
    {
        if (!TypeObj.IsValid()) {
            OutError = TEXT("Invalid entry in 'types' array, expected JSON objects");
            return false;
        }
        
        TArray<FString> Keys;
        TypeObj->Values.GetKeys(Keys);
        if (Keys.Num() != 1)
        {
            OutError = TEXT("Invalid entry in 'types' array, expected single key 'elements' for type kind");
            return false;
        }
        

        SatsKind = SATS::StringToSatsKind(Keys[0]);

        if (SatsKind == SATS::EKind::Invalid)
        {
            OutError = FString::Printf(TEXT("Invalid entry in 'types' array, expected 'Product' or 'Sum', found '%s'"), *Keys[0]);
            return false;
        }

        return true;
    }
}

bool FTypespaceParser::ParseNameAndAlgebraicType(
    const TSharedPtr<FJsonObject>& NameAndAlgTypePair,
    FString &NameString,
    SATS::FAlgebraicKind& AlgebraicOut,
    FString& OutError)
{
    if (!NameAndAlgTypePair.IsValid() || NameAndAlgTypePair->Values.Num() != 2)
    {
        OutError = TEXT("Invalid entry in 'types' TypeDef array, expected JSON objects with two fields: 'name' and 'algebraic_type'");
        return false;
    }
    
    // ---------------------------------
    // Parse field 'name'
    if (!NameAndAlgTypePair->HasTypedField<EJson::String>(TEXT("name")))
    {
        OutError = TEXT("Missing 'name' in TypeDef element");
        return false;
    }
    const TSharedPtr<FJsonObject>& NameObject = NameAndAlgTypePair->GetObjectField(TEXT("name"));
    NameString = NameObject->GetStringField(TEXT("some"));

    // ---------------------------------
    // Parse field 'algebraic_type'
    if (!NameAndAlgTypePair->HasTypedField<EJson::String>(TEXT("algebraic_type")))
    {
        OutError = TEXT("Missing 'algebraic_type' in TypeDef element");
        return false;
    }
    const TSharedPtr<FJsonObject>& AlgebraicTypeObject = NameAndAlgTypePair->GetObjectField(TEXT("algebraic_type"));

    if (!AlgebraicTypeObject->Values.Num() == 1)
    {
        OutError = TEXT("Invalid 'algebraic_type' in TypeDef element");
        return false;
    }
       
    if (!ParseTypespaceAlgebraicType(AlgebraicTypeObject, AlgebraicOut, OutError))
    {
        return false;
    }

    return true;
}

bool FTypespaceParser::ParseTypespaceSum(const TArray<TSharedPtr<FJsonValue>>& Variants,
        SATS::FSumKind& SumOut, FString& OutError)
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

        TSharedPtr<SATS::FAlgebraicKind> AlgebraicType = MakeShared<SATS::FAlgebraicKind>();
        FString NameString;
        if (!ParseNameAndAlgebraicType(OptionObj, NameString, *AlgebraicType, OutError))
        {
            return false;
        }
        
        SumOut.Options.Add({NameString, MoveTemp(AlgebraicType)});
    }
    
    return true;
}

bool FTypespaceParser::ParseTypespaceBuiltin(const TSharedPtr<FJsonObject>& BuiltinObj,
    SATS::FBuiltinKind &BuiltinOut, FString& OutError)
{
    TArray<FString> Keys;
    BuiltinObj->Values.GetKeys(Keys);

    if (Keys.Num() != 1)
    {
        OutError = TEXT("Invalid entry in 'types' array, expected single key for type kind");
        return false;
    }

    const FString BuiltinTypeStr = Keys[0];
    BuiltinOut.Tag = SATS::StringToBuiltinKind(BuiltinTypeStr);
    if (BuiltinOut.Tag == SATS::EBuiltinKind::Array || BuiltinOut.Tag == SATS::EBuiltinKind::Map)
    {
        OutError = FString::Printf(TEXT("SATS parsing of builtin types Array|Map not implemented"));
        return false;
    }

    return true;
}


bool FTypespaceParser::ParseTypespaceAlgebraicType(const TSharedPtr<FJsonObject>& SatsJsonObject,
    SATS::FAlgebraicKind& AlgebraicOut, FString& OutError)
{
    SATS::EKind SatsKind;
    if (!ValidateAndGetSatsKindInTypespaceEntry(SatsJsonObject, SatsKind, OutError))
    {
        return false;
    }
    
    if (SatsKind == SATS::EKind::Product)
    {
        const TArray<TSharedPtr<FJsonValue>>& ProductElements = SatsJsonObject->GetArrayField(TEXT("elements"));
        SATS::FProductKind Product;
        if (!ParseTypespaceProduct(ProductElements, Product, OutError))
        {
            return false;
        }

        AlgebraicOut.Tag = SATS::EKind::Product;
        AlgebraicOut.Product = MoveTemp(Product);
        return true;                    
    }

    if (SatsKind == SATS::EKind::Sum)
    {
        // TODO validate below
        const TArray<TSharedPtr<FJsonValue>>& SumVariants = SatsJsonObject->GetArrayField(TEXT("variants"));
        SATS::FSumKind Sum;
        if (!ParseTypespaceSum(SumVariants, Sum, OutError))
        {
            return false;
        }
        
        AlgebraicOut.Sum = MoveTemp(Sum);
        AlgebraicOut.Tag = SATS::EKind::Sum;
        return true;
    }

    if (SatsKind == SATS::EKind::Ref)
    {
        OutError = TEXT("SATS parsing of Ref types not implemented");
        return false;
    }

    // else it is SATS Builtin 
    // "if SatsKind \in {SATS Builtin}
    {
        const TSharedPtr<FJsonObject>& BuiltinObj = SatsJsonObject;
        SATS::FBuiltinKind Builtin;
        if (!ParseTypespaceBuiltin(BuiltinObj, Builtin, OutError))
        {
            return false;
        }

        // We can safely cast because the enums are correctly mapped
        AlgebraicOut.Tag = static_cast<SATS::EKind>(Builtin.Tag);
        AlgebraicOut.Builtin = MoveTemp(Builtin);
        return true;
    }
}

bool FTypespaceParser::ParseTypespaceProduct(const TArray<TSharedPtr<FJsonValue>>& Elements,
    SATS::FProductKind& ProductOut, FString& OutError)
{
    for (const auto& Element : Elements)
    {
        const TSharedPtr<FJsonObject> ElementObj = Element->AsObject();
    
        if (!ElementObj.IsValid())
        {
            OutError = TEXT("Invalid element in ProductType");
            return false;
        }

        TSharedPtr<SATS::FAlgebraicKind> AlgebraicType = MakeShared<SATS::FAlgebraicKind>();
        FString NameString;
        ParseNameAndAlgebraicType(ElementObj, NameString, *AlgebraicType, OutError);

        
        ProductOut.Elements.Add({NameString, MoveTemp(AlgebraicType)});
    }
    
    return true;
}

bool FTypespaceParser::ParseTypespace(const TSharedPtr<FJsonObject>& RawModuleDefJson,
                                      SATS::FTypespace& TypespaceOutput, FString& OutError)
{
	const TSharedPtr<FJsonObject>* TypespaceObj;
    if (!RawModuleDefJson->TryGetObjectField(TEXT("typespace"), TypespaceObj)) {
        OutError = TEXT("Missing 'typespace' object");
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* TypesArr;
    if (!(*TypespaceObj)->TryGetArrayField(TEXT("types"), TypesArr)) {
        OutError = TEXT("Missing 'typespace.types' array");
        return false;
    }

    TypespaceOutput.TypeEntries.Empty();
    for (auto& TypeVal : *TypesArr) {
        const TSharedPtr<FJsonObject> WrapperObj = TypeVal->AsObject();
        int32 ValueCount = WrapperObj->Values.Num();

        if (bool IsSingleEntry = ValueCount == 1; !IsSingleEntry)
        {
            OutError = FString::Printf(TEXT("Unexpected type entry: expected single key for type kind, found %i"),
                ValueCount);
            return false;
        }
        
        SATS::FTypespace::FAlgebraicDef TypeEntry;
        
        if (WrapperObj->HasField(TEXT("Product")))
        {
            TypeEntry.AlgebraicKind.Tag = SATS::EKind::Product;

            // Check consistency: in SATS, 'Product' is a tag for a single 'elements' Array. 
            const TSharedPtr<FJsonObject> ProductObj = WrapperObj->GetObjectField(TEXT("Product"));
            if (bool IsNotSingleEntry = ProductObj->Values.Num() != 1; IsNotSingleEntry)
            {
                OutError = FString::Printf(TEXT("Unexpected type entry: expected single 'elements' key for "
                                                "ProductType, found %i"), ProductObj->Values.Num());
                return false;
            }
            const TArray<TSharedPtr<FJsonValue>> *ProductTerms;
            if (!ProductObj->TryGetArrayField(TEXT("elements"), ProductTerms))
            {
                
                OutError = TEXT("Unexpected type entry: expected 'elements' key for ProductType, could not find");
                UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
                return false;
            }

            // The list of types in typespace is a bunch of SATS Products or Sums (I guess). We'll only parse Products
            // because that's enough for our quickstart-chat prototype.

            if (!ParseTypespaceProduct(*ProductTerms, TypeEntry.AlgebraicKind.Product, OutError))
            {
                return false;
            }
        }    
        else if (WrapperObj->HasField(TEXT("Sum"))) {
            TypeEntry.AlgebraicKind.Tag = SATS::EKind::Sum;

            // Check consistency: in SATS 'Sum' is a tag for a single 'options' or 'branches' array.
            // For 'branches', we normalize it to 'options'.
            const TSharedPtr<FJsonObject> SumObj = WrapperObj->GetObjectField(TEXT("Sum"));
            const TArray<TSharedPtr<FJsonValue>> *SumTerms;
            
            if (bool IsNotSingleEntry = SumObj->Values.Num() != 1;
                IsNotSingleEntry
                ||
                !(   SumObj->TryGetArrayField(TEXT("options"),  SumTerms)
                  || SumObj->TryGetArrayField(TEXT("branches"), SumTerms)
                  || SumObj->TryGetArrayField(TEXT("variants"), SumTerms)))
            {
                OutError = TEXT("Unexpected type entry: expected single options' or 'branches' array for SumType, found multiple");
                UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
                return false;
            }

            if (!ParseTypespaceSum(*SumTerms, TypeEntry.AlgebraicKind.Sum, OutError))
            {
                return false;
            }
            
        }
        /** TODO: Other SATS types possible here?
         * 
         *  else if (TypeObj->HasField(TEXT("name"))) {
         *      // it’s a Builtin
         *      Entry.Kind = SATS::ETypeKind::Builtin;
         *      Entry.Builtin = TypeObj->GetStringField(TEXT("name"));
         *  }
         *  else if (TypeObj->HasField(TEXT("path"))) {
         *      // it’s a Ref
         *      Entry.Kind = SATS::ETypeKind::Ref;
         *      Entry.Ref.Path = TypeObj->GetStringField(TEXT("path"));
         *  }
         *  
         */ 
        else {
            TArray<FString> OutKeys;
            WrapperObj->Values.GetKeys(OutKeys);
            OutError = FString::Printf(TEXT("Unrecognized type '%s' entry in typespace"), *OutKeys[0]); 
            return false;
        }
        
        TypespaceOutput.TypeEntries.Add(MoveTemp(TypeEntry));
    }

    return true;
}
