#include "ModuleDefParser.h"

#include "TypespaceParser.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Logging/LogMacros.h"
#include "Schema/RawModuleDefSchema.h"

// A few helpers for parsing SATS JSON
namespace 
{
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
    
    // Reads a required JSON array field, returning false + error if missing/not-array
    bool ParseRequiredArray(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field,
                            TArray<TSharedPtr<FJsonValue>>& OutArray, FString& OutError)
    {
        if (!Parent->HasTypedField<EJson::Array>(Field))
        {
            OutError = FString::Printf(TEXT("Missing or invalid array '%s'"), Field);
            return false;
        }
        OutArray = Parent->GetArrayField(Field);
        return true;
    }

    // Reads a required JSON object field, returning nullptr + error if missing/not-object
    TSharedPtr<FJsonObject> ParseRequiredObject(const TSharedPtr<FJsonObject>& Parent,
                                                const TCHAR* Field, FString& OutError)
    {
        if (!Parent->HasField(Field))
        {
            OutError = FString::Printf(TEXT("Missing field '%s'"), Field);
            return nullptr;
        }
        TSharedPtr<FJsonObject> Obj = Parent->GetObjectField(Field);
        if (!Obj.IsValid())
        {
            OutError = FString::Printf(TEXT("Field '%s' is not a JSON object"), Field);
            return nullptr;
        }
        return Obj;
    }

    // Unwraps an Option<String> JSON object ({ some: val } or { none: [] })
    FString UnwrapOptionString(const TSharedPtr<FJsonObject>& OptionObj)
    {
        const FString SomeKey = TEXT("some");
        return OptionObj->HasField(SomeKey)
            ? OptionObj->GetStringField(SomeKey)
            : FString();
    }

    // Parses a single-entry SATS sum under a given field into Tag/Payload
    bool ParseSingleEntrySum(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field,
                             SATS::FReducerDef::FReturnType& OutType, FString& OutError)
    {
        TSharedPtr<FJsonObject> SumObj = ParseRequiredObject(Parent, Field, OutError);
        if (!SumObj) return false;
        if (SumObj->Values.Num() != 1)
        {
            OutError = FString::Printf(TEXT("'%s' must have exactly one type tag"), Field);
            return false;
        }
        auto It = SumObj->Values.CreateConstIterator();
        OutType.Tag     = It->Key;
        OutType.Payload = It->Value;
        return true;
    }
    
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

bool FModuleDefParser::ParseTables(
    const TSharedPtr<FJsonObject>& RawModuleDefJson,
    TArray<SATS::FTableDef>& TablesOutput,
    FString& OutError)
{
    if (const TArray<TSharedPtr<FJsonValue>>* Tables;
        RawModuleDefJson->TryGetArrayField(TEXT("tables"), Tables)) {
        TablesOutput.Empty();
        for (auto& V : *Tables) {
            auto Obj = V->AsObject();
            SATS::FTableDef T;
            T.Name = Obj->GetStringField(TEXT("name"));
            T.ProductTypeRef = Obj->GetIntegerField(TEXT("product_type_ref"));
            // primary_key
            const TArray<TSharedPtr<FJsonValue>>* PrimaryKeyArray;
            if (Obj->TryGetArrayField(TEXT("primary_key"), PrimaryKeyArray)) {
                for (auto& P : *PrimaryKeyArray) {
                    T.PrimaryKey.Add(P->AsString());
                }
            }
            // TODO: parse indexes, constraints, sequences, schedule, access
            TablesOutput.Add(MoveTemp(T));
        }
    }
    
    return true;
}

bool FModuleDefParser::ParseReducers(
    const TSharedPtr<FJsonObject>& RawModuleDefJson,
    TArray<SATS::FReducerDef> &ReducersOutput,
    FString& OutError)
{
    // Clear any previous error state
    OutError.Empty();

    // Top‐level 'reducers' array
    TArray<TSharedPtr<FJsonValue>> ReducersArray;
    if (!ParseRequiredArray(RawModuleDefJson, TEXT("reducers"), ReducersArray, OutError))
    {
        return false;
    }

    ReducersOutput.Reserve(ReducersArray.Num());
    for (const auto& ReducerValue : ReducersArray)
    {
        TSharedPtr<FJsonObject> ReducerObj = ReducerValue->AsObject();
        if (!ReducerObj.IsValid())
        {
            OutError = TEXT("Invalid reducer entry; expected JSON object");
            return false;
        }

        SATS::FReducerDef ReducerDef;

        // Name
        {
            TSharedPtr<FJsonObject> NameField = ParseRequiredObject(ReducerObj, TEXT("name"), OutError);
            if (!NameField) return false;
            ReducerDef.Name = UnwrapOptionString(NameField);
        }

        // Params (inline Product)
        {
            if (ReducerObj->HasField(TEXT("params")))
            {
                TSharedPtr<FJsonObject> ParamsObj = ParseRequiredObject(ReducerObj, TEXT("params"), OutError);
                if (!ParamsObj) return false;

                TArray<TSharedPtr<FJsonValue>> Elements;
                if (!ParseRequiredArray(ParamsObj, TEXT("elements"), Elements, OutError))
                {
                    return false;
                }
                for (const auto& ElemVal : Elements)
                {
                    TSharedPtr<FJsonObject> ElemObj = ElemVal->AsObject();
                    if (!ElemObj.IsValid())
                    {
                        OutError = FString::Printf(TEXT("Invalid element in params for reducer '%s'"),
                            *ReducerDef.Name);
                        return false;
                    }
                    // name unwrap
                    TSharedPtr<FJsonObject> NameObj = ParseRequiredObject(ElemObj, TEXT("name"), OutError);
                    if (!NameObj) return false;
                    FString ParamName = UnwrapOptionString(NameObj);

                    // type tag
                    SATS::FReducerDef::FReturnType ParamType;
                    if (!ParseSingleEntrySum(ElemObj, TEXT("algebraic_type"), ParamType, OutError))
                    {
                        return false;
                    }

                    // add field
                    auto AlgebraicType = MakeShared<SATS::FAlgebraicKind>();
                    if (!FTypespaceParser::ParseTypespaceAlgebraicType(ElemObj, *AlgebraicType, OutError))
                    {
                        return false;
                    }
                    ReducerDef.Params.Elements.Add({ParamName, AlgebraicType});
                }
            }
        }

        // Do Reducers have return type?

        ReducersOutput.Add(MoveTemp(ReducerDef));
    }

    return true;
}

bool FModuleDefParser::ParseRawModuleDef(
    const TSharedPtr<FJsonObject>& RawModuleDefJson,
    SATS::FRawModuleDef& OutDef,
    FString& OutError)
{
    // --- typespace ---
    if (!FTypespaceParser::ParseTypespace(RawModuleDefJson, OutDef.Typespace, OutError))
    {
        return false;
    }

    // --- tables ---
    if (!ParseTables(RawModuleDefJson, OutDef.Tables, OutError))
    {
        return false;
    }

    // --- reducers ---
    if (!ParseReducers(RawModuleDefJson, OutDef.Reducers, OutError))
    {
        return false;   
    }

    // We can continue parsing 'types', 'misc_exports', 'row_level_security' similarly

    return true;
}



