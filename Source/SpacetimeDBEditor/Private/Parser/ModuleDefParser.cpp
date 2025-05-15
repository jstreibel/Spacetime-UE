#include "ModuleDefParser.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Logging/LogMacros.h"

bool FModuleDefParser::Parse(
    const FString& RawJson,
    TArray<FTableSchema>& OutTables,
    TArray<FReducerSchema>& OutReducers,
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

    // 1. Parse tables
    const TArray<TSharedPtr<FJsonValue>>* TablesJson = nullptr;
    if (!Root->TryGetArrayField(TEXT("tables"), TablesJson))
    {
        OutError = TEXT("'tables' array missing in module definition.");
        UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
        return false;
    }

    for (const TSharedPtr<FJsonValue>& TableVal : *TablesJson)
    {
        const TSharedPtr<FJsonObject> TblObj = TableVal->AsObject();
        FTableSchema Table;
        Table.TableName = TblObj->GetStringField(TEXT("name"));

        // TODO: parse actual columns via typespace; leave empty for now
        OutTables.Add(MoveTemp(Table));
    }

    // 2. Parse reducers
    const TArray<TSharedPtr<FJsonValue>>* ReducersJson = nullptr;
    if (Root->TryGetArrayField(TEXT("reducers"), ReducersJson))
    {
        for (const TSharedPtr<FJsonValue>& ReducerVal : *ReducersJson)
        {
            const TSharedPtr<FJsonObject> RedObj = ReducerVal->AsObject();
            FReducerSchema Reducer;
            Reducer.ReducerName = RedObj->GetStringField(TEXT("name"));

            // Parse 'params' Product elems
            const TSharedPtr<FJsonObject>* ParamsObj;
            if (RedObj->TryGetObjectField(TEXT("params"), ParamsObj))
            {
                const TSharedPtr<FJsonObject>* ProductObj;
                if ((*ParamsObj)->TryGetObjectField(TEXT("Product"), ProductObj))
                {
                    const TArray<TSharedPtr<FJsonValue>>* Elements = nullptr;
                    if ((*ProductObj)->TryGetArrayField(TEXT("elements"), Elements))
                    {
                        for (const TSharedPtr<FJsonValue>& ElemVal : *Elements)
                        {
                            const TSharedPtr<FJsonObject> ElemObj = ElemVal->AsObject();
                            // Name
                            const TSharedPtr<FJsonObject> NameObj = ElemObj->GetObjectField(TEXT("name"));
                            FString ParamName = NameObj->GetStringField(TEXT("some"));

                            // Type
                            const TSharedPtr<FJsonObject> TypeObj = ElemObj->GetObjectField(TEXT("algebraic_type"));
                            FString UEType = TEXT("FString");
                            for (const auto& KV : TypeObj->Values)
                            {
                                const FString& Key = KV.Key;
                                if (Key == TEXT("String"))      { UEType = TEXT("FString"); }
                                else if (Key == TEXT("Int"))    { UEType = TEXT("int32"); }
                                else if (Key == TEXT("Float"))  { UEType = TEXT("float"); }
                                else if (Key == TEXT("Bool"))   { UEType = TEXT("bool"); }
                                // TODO: handle arrays, custom types, etc.
                                break;
                            }

                            Reducer.Params.Add({ MoveTemp(ParamName), MoveTemp(UEType), false });
                        }
                    }
                }
            }

            OutReducers.Add(MoveTemp(Reducer));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Parsed %d tables and %d reducers."), OutTables.Num(), OutReducers.Num());
    return true;
}
