#include "TypespaceParser.h"

#include "Common.h"

bool FTypespaceParser::ParseTypespace(const TSharedPtr<FJsonObject>& RawModuleDefJson,
                                      SATS::FTypespace& TypespaceOutput, FString& OutError)
{
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Parsing typespace"));
    
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
        const int32 ValueCount = WrapperObj->Values.Num();

        if (const bool IsSingleEntry = ValueCount == 1; !IsSingleEntry)
        {
            OutError = FString::Printf(TEXT("Unexpected type entry: expected single key for type kind, found %i"),
                ValueCount);
            return false;
        }
        
        SATS::FAlgebraicType TypeEntry;
        
        if (WrapperObj->HasField(TEXT("Product")))
        {
            TypeEntry.Tag = SATS::EType::Product;

            // Check consistency: in SATS, 'Product' is a tag for a single 'elements' Array. 
            const TSharedPtr<FJsonObject> ProductObj = WrapperObj->GetObjectField(TEXT("Product"));
            if (const bool IsNotSingleEntry = ProductObj->Values.Num() != 1; IsNotSingleEntry)
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

            // The list of Algebraic Types in typespace is described by a bunch of SATS Algebraic Values.
            if (!FCommon::ParseProduct(*ProductTerms, TypeEntry.Product, OutError))
            {
                int TypeIndex = TypespaceOutput.TypeEntries.Num();
                OutError = FString::Printf(TEXT("Failed to parse Product Type %i: "), TypeIndex) + OutError;
                return false;
            }
        }    
        else if (WrapperObj->HasField(TEXT("Sum"))) {
            TypeEntry.Tag = SATS::EType::Sum;

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

            if (!FCommon::ParseSum(*SumTerms, TypeEntry.Sum, OutError))
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
