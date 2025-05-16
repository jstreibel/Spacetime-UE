#include "ModuleDefParser.h"

#include "ToolMenusEditor.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Logging/LogMacros.h"
#include "Schema/RawModuleDefSchema.h"

// Converts any snake_case, kebab-case, space separated, or camelCase string
// into PascalCase (e.g. "chat_message" → "ChatMessage", "sendMessage" → "SendMessage").
auto ToPascalCase = [](const FString& InString) -> FString
{
    FString Result;
    bool bCapNext = true;

    const int32 Len = InString.Len();
    for (int32 i = 0; i < Len; ++i)
    {
        const TCHAR Ch = InString[i];

        if (FChar::IsAlnum(Ch))
        {
            // If we see an uppercase in the middle of a word that follows a lowercase,
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

static FString MapJsonTypeToUE(const FString& JsonType)
{
    // 1) Primitive JSON → UE types
    static const TMap<FString, FString> PrimitiveMap = {
        { TEXT("integer"),  TEXT("int32") },
        { TEXT("number"),   TEXT("double") },
        { TEXT("float"),    TEXT("float") },
        { TEXT("double"),   TEXT("double") },
        { TEXT("string"),   TEXT("FString") },
        { TEXT("boolean"),  TEXT("bool") },
        { TEXT("uuid"),     TEXT("FGuid") },
        { TEXT("datetime"), TEXT("FDateTime") },
        { TEXT("bytes"),    TEXT("TArray<uint8>") }
    };

    if (const FString* Found = PrimitiveMap.Find(JsonType))
    {
        return *Found;
    }

    // 2) Array types: e.g. "array<string>"
    const FString ArrayPrefix = TEXT("array<");
    if (JsonType.StartsWith(ArrayPrefix) && JsonType.EndsWith(TEXT(">")))
    {
        // Extract inner JSON type
        int32 InnerLen = JsonType.Len() - ArrayPrefix.Len() - 1;
        FString InnerJsonType = JsonType.Mid(ArrayPrefix.Len(), InnerLen);
        // Recursively map inner type
        FString InnerUEType = MapJsonTypeToUE(InnerJsonType);
        return FString::Printf(TEXT("TArray<%s>"), *InnerUEType);
    }

    // 3) Fallback for object/custom types:
    //    Prefix with 'F' and convert to PascalCase (e.g. "user_profile" → "FUserProfile")
    FString PascalName = ToPascalCase(JsonType);
    return FString::Printf(TEXT("F%s"), *PascalName);
}


bool FModuleDefParser::Parse(
    const FString& RawJson,
    SATS::FRawModuleDef &RawModule,
    FString& OutError
)
{
    // Deserialize into a JSON object
    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawJson);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        OutError = TEXT("Failed to parse JSON module definition.");
        UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
        return false;
    }

    return ParseRawModuleDef(Root, RawModule, OutError);
}

bool FModuleDefParser::ParseRawModuleDef(
    const TSharedPtr<FJsonObject>& RawModuleDefJson,
    SATS::FRawModuleDef& OutDef,
    FString& OutError)
{
    // Helper to read a string
    auto ReadString = [&](const FString& Key, FString& Dest) -> bool {
        if (RawModuleDefJson->HasField(Key)) {
            Dest = RawModuleDefJson->GetStringField(Key);
            return true;
        }
        OutError = FString::Printf(TEXT("Missing field '%s'"), *Key);
        return false;
    };

    // --- typespace ---
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

    OutDef.Typespace.Types.Empty();
    for (auto& TypeVal : *TypesArr) {
        const TSharedPtr<FJsonObject> WrapperObj = TypeVal->AsObject();
        int32 ValueCount = WrapperObj->Values.Num();

        if (bool IsSingleEntry = ValueCount == 1; !IsSingleEntry)
        {
            OutError = FString::Printf(TEXT("Unexpected type entry: expected single key for type kind, found %i"),
                ValueCount);
            return false;
        }
        
        SATS::FTypeEntry Entry;
        
        if (WrapperObj->HasField(TEXT("Product")))
        {
            Entry.Kind = SATS::ETypeKind::Product;

            // Check consistency: in SATS, 'Product' is a tag for a single 'elements' Array. 
            const TSharedPtr<FJsonObject> ProductObj = WrapperObj->GetObjectField(TEXT("Product"));
            if (bool IsNotSingleEntry = ProductObj->Values.Num() != 1; IsNotSingleEntry)
            {
                OutError = FString::Printf(TEXT("Unexpected type entry: expected single 'elements' key for "
                                                "ProductType, found %i"), ProductObj->Values.Num());
                UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
                return false;
            }
            const TArray<TSharedPtr<FJsonValue>> *ProductTerms;
            if (!ProductObj->TryGetArrayField(TEXT("elements"), ProductTerms))
            {
                
                OutError = TEXT("Unexpected type entry: expected 'elements' key for ProductType, could not find");
                UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
                return false;
            }
            
            ParseTypespaceProductType(*ProductTerms, Entry.Product, OutError);
        }    
        else if (WrapperObj->HasField(TEXT("Sum"))) {
            Entry.Kind = SATS::ETypeKind::Sum;

            // Check consistency: in SATS 'Sum' is a tag for a single 'options' or 'branches' array.
            // For 'branches', we normalize it to 'options'.
            const TSharedPtr<FJsonObject> SumObj = WrapperObj->GetObjectField(TEXT("Sum"));
            const TArray<TSharedPtr<FJsonValue>> *SumTerms;
            
            if (bool IsNotSingleEntry = SumObj->Values.Num() != 1;
                IsNotSingleEntry
                ||
                !(     SumObj->TryGetArrayField(TEXT("options"), SumTerms)
                    || SumObj->TryGetArrayField(TEXT("branches"), SumTerms) ))
            {
                OutError = TEXT("Unexpected type entry: expected single options' or 'branches' array for SumType, found multiple");
                UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
                return false;
            }
            
            ParseTypespaceSumType(*SumTerms, Entry.Sum, OutError);
        }
        /*
        else if (TypeObj->HasField(TEXT("name"))) {
            // it’s a Builtin
            Entry.Kind = SATS::ETypeKind::Builtin;
            Entry.Builtin = TypeObj->GetStringField(TEXT("name"));
        }
        else if (TypeObj->HasField(TEXT("path"))) {
            // it’s a Ref
            Entry.Kind = SATS::ETypeKind::Ref;
            Entry.Ref.Path = TypeObj->GetStringField(TEXT("path"));
        }*/ 
        else {
            TArray<FString> OutKeys;
            WrapperObj->Values.GetKeys(OutKeys);
            OutError = FString::Printf(TEXT("Unrecognized type '%s' entry in typespace"), *OutKeys[0]); 
            return false;
        }

        
        OutDef.Typespace.Types.Add(MoveTemp(Entry));
    }

    // --- tables ---
    if (const TArray<TSharedPtr<FJsonValue>>* Tables; RawModuleDefJson->TryGetArrayField(TEXT("tables"), Tables)) {
        OutDef.Tables.Empty();
        for (auto& V : *Tables) {
            auto Obj = V->AsObject();
            SATS::FTableDef T;
            T.Name = Obj->GetStringField(TEXT("name"));
            T.ProductTypeRef = Obj->GetIntegerField(TEXT("product_type_ref"));
            // primary_key
            const TArray<TSharedPtr<FJsonValue>>* PK;
            if (Obj->TryGetArrayField(TEXT("primary_key"), PK)) {
                for (auto& P : *PK) {
                    T.PrimaryKey.Add(P->AsString());
                }
            }
            // TODO: parse indexes, constraints, sequences, schedule, access
            OutDef.Tables.Add(MoveTemp(T));
        }
    }

    // --- reducers ---
    const TArray<TSharedPtr<FJsonValue>>* Reducers;
    if (RawModuleDefJson->TryGetArrayField(TEXT("reducers"), Reducers)) {
        OutDef.Reducers.Empty();
        for (auto& V : *Reducers) {
            auto Obj = V->AsObject();
            SATS::FReducerDef R;
            R.Name = Obj->GetStringField(TEXT("name"));
            // params
            const TSharedPtr<FJsonObject>* ParamObj;
            if (Obj->TryGetObjectField(TEXT("params"), ParamObj)) {
                const TArray<TSharedPtr<FJsonValue>>* Elems;
                (*ParamObj)->TryGetArrayField(TEXT("elements"), Elems);
                for (auto& E : *Elems) {
                    auto EO = E->AsObject();
                    SATS::FProductType::FField F;
                    F.Name = EO->GetStringField(TEXT("name"));
                    F.TypeRef = EO->GetIntegerField(TEXT("type_ref"));
                    R.Params.Elements.Add(F);
                }
            }
            // lifecycle same as SumType parsing
            // ...
            OutDef.Reducers.Add(MoveTemp(R));
        }
    }

    // We can continue parsing 'types', 'misc_exports', 'row_level_security' similarly

    return true;
}

bool FModuleDefParser::ParseTypespaceProductType(
    const TArray<TSharedPtr<FJsonValue>> &ProductTermsArray,
    SATS::FProductType& OutProduct,
    FString& OutError
) {
    // Clear any existing elements
    OutProduct.Elements.Empty();

    // Parse each element object (term of the product)
    for (const auto& Value : ProductTermsArray) {
        if (!Value.IsValid() || !Value->AsObject().IsValid()) {
            OutError = TEXT("Invalid entry in 'elements' array, expected JSON objects");
            return false;
        }
        const TSharedPtr<FJsonObject> ElemObj = Value->AsObject();

        // Parse field name
        if (!ElemObj->HasTypedField<EJson::String>(TEXT("name"))) {
            OutError = TEXT("Missing 'name' in ProductType element");
            return false;
        }
        FString FieldName = ElemObj->GetStringField(TEXT("name"));

        // Parse type reference
        if (!ElemObj->HasTypedField<EJson::Number>(TEXT("type_ref"))) {
            OutError = TEXT("Missing 'type_ref' in ProductType element");
            return false;
        }
        int32 TypeRef = ElemObj->GetIntegerField(TEXT("type_ref"));

        // Construct and add the field
        SATS::FProductType::FField Field;
        Field.Name = MoveTemp(FieldName);
        Field.TypeRef = TypeRef;
        OutProduct.Elements.Add(MoveTemp(Field));
    }

    return true;
}

bool FModuleDefParser::ParseTypespaceSumType(
    const TArray<TSharedPtr<FJsonValue>> &SumTerms,
    SATS::FSumType& OutSum,
    FString& OutError
) {
    // Clear any existing options
    OutSum.Options.Empty();

    // Parse each option object
    for (const auto& Value : SumTerms) {
        if (!Value.IsValid() || !Value->AsObject().IsValid()) {
            OutError = TEXT("Invalid entry in 'options' array, expected JSON objects");
            return false;
        }
        const TSharedPtr<FJsonObject> OptObj = Value->AsObject();

        // Parse tag
        if (!OptObj->HasTypedField<EJson::String>(TEXT("tag"))) {
            OutError = TEXT("Missing 'tag' in SumType option");
            return false;
        }
        FString Tag = OptObj->GetStringField(TEXT("tag"));

        // Parse type reference
        if (!OptObj->HasTypedField<EJson::Number>(TEXT("type_ref"))) {
            OutError = TEXT("Missing 'type_ref' in SumType option");
            return false;
        }
        int32 TypeRef = OptObj->GetIntegerField(TEXT("type_ref"));

        // Construct and add the branch
        SATS::FSumType::FBranch Branch;
        Branch.Tag = MoveTemp(Tag);
        Branch.TypeRef = TypeRef;
        OutSum.Options.Add(MoveTemp(Branch));
    }

    return true;
}
