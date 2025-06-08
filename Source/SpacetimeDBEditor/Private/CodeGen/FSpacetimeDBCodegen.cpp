#include "FSpacetimeDBCodegen.h"

#include "Containers/UnrealString.h"
#include "Net/RepLayout.h"
#include "Parser/Common.h"

const FString ApiMacroString = TEXT("SPACETIMEDBRUNTIME_API");  

FString FSpacetimeDBCodeGen::MapBuiltinToUnreal(const FString& BuiltinName)
{
    if (BuiltinName == "Bool")         return "bool";
    if (BuiltinName == "I8")           return "int8";
    if (BuiltinName == "U8")           return "uint8";
    if (BuiltinName == "I16")          return "int16";
    if (BuiltinName == "U16")          return "uint16";
    if (BuiltinName == "I32")          return "int32";
    if (BuiltinName == "U32")          return "uint32";
    if (BuiltinName == "I64")          return "int64";
    if (BuiltinName == "U64")          return "uint64";
    if (BuiltinName == "I128")         return "int128";
    if (BuiltinName == "U128")         return "uint128";
    if (BuiltinName == "F32")          return "float";
    if (BuiltinName == "F64")          return "double";
    if (BuiltinName == "String")       return "FString";
    if (BuiltinName == "Array")        return "// TArray<...>";
    if (BuiltinName == "Map")          return "// TMap<...>";
    
    return FString::Printf(TEXT("// unknown SATS BuiltIn '%s'"), *BuiltinName);
}

inline bool IsBuiltinWithNativeRepresentation(const FString& TypeName)
{
    const TSet<FString> BuiltinsWithNativeRepresentations = {
        "Bool",
        "I8",
        "U8",
        "I16",
        "U16",
        "I32",
        "U32",
        "I64",
        "U64",
        "I128",
        "U128",
        "F32",
        "F64",
        "String"
    };
    
    return BuiltinsWithNativeRepresentations.Contains(TypeName);
}

inline bool IsBuiltinWithNativeRepresentation(const SATS::EType& Kind)
{
    return IsBuiltinWithNativeRepresentation(SATS::TypeToString(Kind));
}

// Converts any snake_case, kebab-case, space separated, or camelCase string
// into PascalCase (e.g. "chat_message" → "ChatMessage", "sendMessage" → "SendMessage").
FString FSpacetimeDBCodeGen::ToPascalCase(const FString& InString)
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
}

FString FSpacetimeDBCodeGen::ResolveAlgebraicTypeToUnrealCxx(const SATS::FAlgebraicType& AlgebraicKind)
{
    switch (AlgebraicKind.Tag)
    {
    case SATS::EType::Product:  return "// Product placeholder";
    case SATS::EType::Sum:      return "// Sum placeholder";
    case SATS::EType::Ref:      return "// Ref placeholder";

    // case BuiltIn:
    case SATS::EType::Array:    return TEXT("// TArray placeholder");
    case SATS::EType::Map:      return TEXT("// TMap placeholder");
    default:   /* BuiltIn */    return MapBuiltinToUnreal(SATS::TypeToString(AlgebraicKind.Tag));
    }    
}

bool FSpacetimeDBCodeGen::GenerateTableStructs(
    const SATS::FRawModuleDef& ModuleDef,
    const FString& HeaderName,
    FString& OutHeader,
    FString& OutError)
{
    FString OutHeaderText;
    OutHeaderText += TEXT("#pragma once\n\n"
                   "#include \"CoreMinimal.h\"\n"
                   "#include \"UObject/NoExportTypes.h\"\n"
                   "#include \"" + HeaderName + ".generated.h\"\n\n\n");

    for (const auto& Table : ModuleDef.Tables)
    {
        // Lookup product definition
        const int ProductTypeRef = Table.ProductTypeRef;
        const auto& ProductType = ModuleDef.Typespace.TypeEntries[ProductTypeRef];
        if (ProductType.AlgebraicType.Tag != SATS::EType::Product)
        {
            OutError = TEXT("Table type is expected to be a SATS Product type.");
            return false;
        }

        FString StructName = TEXT("F") + ToPascalCase(Table.Name) + TEXT("Row");
        OutHeaderText += TEXT("USTRUCT(BlueprintType)");
        OutHeaderText += TEXT("\nstruct ") + ApiMacroString + " " + StructName + TEXT(" {\n\n"
            "    GENERATED_BODY()\n\n");

        /*
        for (const auto& [Name, AlgebraicKind] : ProductElements)
        {
            FString CxxTypeString = ResolveAlgebraicTypeToUnrealCxx(AlgebraicKind);
            
            // FString CxxTypeString = MapBuiltin(ModuleDef.Typespace.TypeEntries[Type_Ref].Builtin); 
            if (!IsBuiltinWithNativeRepresentation(AlgebraicKind->Tag))
            {
                Implement
            }
            else
            {
                Implement
            }
            
            FString Prop = FString::Printf(
                TEXT("    UPROPERTY(BlueprintReadWrite) %s %s;\n"),
                *CxxTypeString, *ToPascalCase(Name)
            );
            
            OutHeaderText += Prop;
        }
        OutHeaderText += TEXT("};\n\n\n");
        */
    }
    // OutHeader = MoveTemp(OutHeaderText);
    // return true;
    
    OutError = TEXT("Tables codegen not implemented");
    return false;
}

bool FSpacetimeDBCodeGen::GenerateReducerFunctions(
    const SATS::FRawModuleDef& ModuleDef,
    const FString& HeaderName,
    FString& OutHeader,
    FString& OutSource,
    FString& OutError
)
{
    
    // Header
    FString HeaderText;
    HeaderText += TEXT("#pragma once\n\n"
                "#include \"Kismet/BlueprintFunctionLibrary.h\"\n"
                "#include \"CoreMinimal.h\"\n"
                "#include \"" + HeaderName + ".generated.h" + "\"\n\n\n");
    HeaderText += TEXT("UCLASS()\n"
                "class " + ApiMacroString + " USpacetimeDBReducers : public UBlueprintFunctionLibrary {\n\n"
                "   GENERATED_BODY()\n\npublic:\n\n");

    // Source
    FString Src;
    Src += TEXT("#include \"Generated/" + HeaderName + ".h\"\n\n\n");

    for (const auto& ReducerDef : ModuleDef.Reducers)
    {
        // Function signature
        TArray<FString> Params;
        for (const auto& [Name, AlgebraicType] : ReducerDef.Params)
        {            
            // FString UEType = MapBuiltinToUnreal(ModuleDef.Typespace.TypeEntries[Argument.TypeRef].Builtin);
            FString ArgName = Name.IsSet() ? ToPascalCase(*Name) : FCommon::CreateUniqueName();

            if (!IsBuiltinWithNativeRepresentation(AlgebraicType.Tag))
            {
                OutError = TEXT("[prototype error] reducer argument is expected to be a SATS BuiltIn type in prototype.");
            }

            FString UEType = ResolveAlgebraicTypeToUnrealCxx(AlgebraicType);
            
            const FString ParamArgString = FString::Printf(TEXT("const %s& %s"), *UEType, *ArgName);
            
            Params.Add(ParamArgString);
        }
        
        FString Sig = FString::Printf(
            TEXT("    UFUNCTION(BlueprintCallable, Category=\"SpacetimeDB\")\n    static void %s(%s);\n\n"),
            *ToPascalCase(ReducerDef.Name), *FString::Join(Params, TEXT(", "))
        );
        HeaderText += Sig;

        // Implementation stub
        FString Impl = FString::Printf(
            TEXT("void USpacetimeDBReducers::%s(%s)\n{\n    // TODO: call SpacetimeDB client reducer '%s'\n}\n\n"),
            *ToPascalCase(ReducerDef.Name), *FString::Join(Params, TEXT(", ")),
            *ReducerDef.Name
        );
        Src += "\n" + Impl;
    }
    HeaderText += TEXT("};\n");

    OutHeader = MoveTemp(HeaderText);
    OutSource = MoveTemp(Src);
    return true;
}

bool FSpacetimeDBCodeGen::GenerateTypespaceStructs(
    const SATS::FRawModuleDef& ModuleDef,
    const FString& HeaderName,
    FString& OutHeader,
    FString& OutError)
{
    for (const auto &TypeEntry : ModuleDef.Typespace.TypeEntries)
    {
        FString OutHeaderText;
        OutHeaderText += TEXT("#pragma once\n\n"
                       "#include \"CoreMinimal.h\"\n"
                       "#include \"UObject/NoExportTypes.h\"\n"
                       "#include \"" + HeaderName + ".generated.h\"\n\n\n");

        FString StructName = TEXT("F") + ToPascalCase(TypeEntry.Name);
        OutHeaderText += TEXT("USTRUCT(BlueprintType)");
        OutHeaderText += TEXT("\nstruct ") + ApiMacroString + " " + StructName + TEXT(" {\n\n"
                "    GENERATED_BODY()\n\n");
        
        if (TypeEntry.AlgebraicType.Tag == SATS::EType::Product)
        {
            
        }
        //for (Ty)

        /*
        FString Prop = FString::Printf(
            TEXT("    UPROPERTY(BlueprintReadWrite) %s %s;\n"),
            *CxxTypeString, *ToPascalCase(Name)
        ); 
        
        OutHeaderText += Prop;
        */
        

        OutHeaderText += TEXT("};\n\n\n");
        
        // OutHeader = MoveTemp(OutHeaderText);
        // return true;
    }

    return true;
}
