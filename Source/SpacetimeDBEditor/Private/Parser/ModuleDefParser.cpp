#include "ModuleDefParser.h"

#include "Common.h"
#include "TypespaceParser.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Logging/LogMacros.h"
#include "Schema/RawModuleDefSchema.h"

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
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Parsing tables"));
    
    if (const TArray<TSharedPtr<FJsonValue>>* Tables;
        RawModuleDefJson->TryGetArrayField(TEXT("tables"), Tables)) {
        TablesOutput.Empty();
        for (auto& V : *Tables) {
            const auto Obj = V->AsObject();
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

        return true;
    }

    OutError = TEXT("Missing or invalid 'tables' array");
    return false;
}

bool FModuleDefParser::ParseReducers(
    const TSharedPtr<FJsonObject>& RawModuleDefJson,
    TArray<SATS::FReducerDef> &ReducersOutput,
    FString& OutError)
{
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Parsing reducers"));
    
    // Clear any previous error state
    OutError.Empty();

    // Top‐level 'reducers' array
    TArray<TSharedPtr<FJsonValue>> ReducersArray;
    if (!FCommon::ParseRequiredArray(RawModuleDefJson, TEXT("reducers"), ReducersArray, OutError))
    {
        OutError = TEXT("Missing or invalid 'reducers' array");
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
            FString NameField;
            if (!FCommon::ParseRequiredString(ReducerObj, TEXT("name"), NameField, OutError))
            {
                return false;
            }
            ReducerDef.Name =  NameField;
        }

        // Params (inline Product)
        {
            TSharedPtr<FJsonObject> ParamsObj = FCommon::ParseRequiredObject(ReducerObj, TEXT("params"), OutError);
            if (!ParamsObj) return false;

            TArray<TSharedPtr<FJsonValue>> ParamsValueArray;
            if (!FCommon::ParseRequiredArray(ParamsObj, TEXT("elements"), ParamsValueArray, OutError))
            {
                return false;
            }
            
            for (const TSharedPtr<FJsonValue>& ParamValue : ParamsValueArray)
            {
                TSharedPtr<FJsonObject> ParamObj = ParamValue->AsObject();
                if (!ParamObj.IsValid())
                {
                    OutError = FString::Printf(TEXT("Invalid element in params for reducer '%s'"),
                        *ReducerDef.Name);
                    return false;
                }

                SATS::FOptionalString ParamName;
                SATS::FAlgebraicType AlgebraicType;
                if (!FCommon::ParseNameAndAlgebraicType(ParamObj, ParamName, AlgebraicType, OutError))
                {
                    return false;
                }

                // add field
                ReducerDef.Params.Add({ParamName, AlgebraicType});
            }
        }

        // TODO: return_type_ref
        // TODO: type tag

        ReducersOutput.Add(MoveTemp(ReducerDef));
    }

    return true;
}

bool FModuleDefParser::ParseRawModuleDef(
    const TSharedPtr<FJsonObject>& RawModuleDefJson,
    SATS::FRawModuleDef& OutDef,
    FString& OutError)
{
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Parsing module definition"));
    
    // --- typespace ---
    if (!FTypespaceParser::ParseTypespace(RawModuleDefJson, OutDef.Typespace, OutError))
    {
        OutError = TEXT("Failed to parse typespace: ") + OutError;
        return false;
    }

    // --- tables ---
    if (!ParseTables(RawModuleDefJson, OutDef.Tables, OutError))
    {
        OutError = TEXT("Failed to parse tables: ") + OutError;
        return false;
    }

    // --- reducers ---
    if (!ParseReducers(RawModuleDefJson, OutDef.Reducers, OutError))
    {
        OutError = TEXT("Failed to parse reducers: ") + OutError;
        return false;   
    }

    // We can continue parsing 'types', 'misc_exports', 'row_level_security' similarly

    return true;
}



