#include "FSpacetimeDBCodegen.h"

#include "Containers/UnrealString.h"
#include "Net/RepLayout.h"

const FString ApiMacroString = TEXT("SPACETIMEDB_API");  

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

inline bool IsBuiltinWithNativeRepresentation(const SATS::EKind& Kind)
{
    return IsBuiltinWithNativeRepresentation(SATS::KindToString(Kind));
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

FString FSpacetimeDBCodeGen::ResolveAlgebraicTypeToUnrealCxx(const TSharedPtr<SATS::FAlgebraicKind>& AlgebraicKind)
{
    switch (AlgebraicKind->Tag)
    {
    case SATS::EKind::Product:  return "// Product placeholder";
    case SATS::EKind::Sum:      return "// Sum placeholder";
    case SATS::EKind::Ref:      return "// Ref placeholder";

    // case BuiltIn:
    case SATS::EKind::Array:    return TEXT("// TArray placeholder");
    case SATS::EKind::Map:      return TEXT("// TMap placeholder");
    default:        
    }

    return MapBuiltinToUnreal(SATS::KindToString(AlgebraicKind->Tag));
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
        if (ProductType.AlgebraicKind.Tag != SATS::EKind::Product)
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
    
    OutError = TEXT("Not implemented");
    return false;
}

bool FSpacetimeDBCodeGen::GenerateReducerFunctions(
    const SATS::FRawModuleDef& ModuleDef,
    const FString& HeaderName,
    FString& OutHeader,
    FString& OutSource
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
        for (const auto& Field : ReducerDef.Params.Elements)
        {
            FString UEType = MapBuiltinToUnreal(ModuleDef.Typespace.TypeEntries[Field.TypeRef].Builtin);
            FString ArgName = ToPascalCase(Field.Name);
            Params.Add(FString::Printf(TEXT("const %s& %s"), *UEType, *ArgName));
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
        
        if (TypeEntry.AlgebraicKind.Tag == SATS::EKind::Product)
        {
            
        }
        //for (Ty)
        
        FString Prop = FString::Printf(
            TEXT("    UPROPERTY(BlueprintReadWrite) %s %s;\n"),
            *CxxTypeString, *ToPascalCase(Name)
        );
        
        OutHeaderText += Prop;
        

        OutHeaderText += TEXT("};\n\n\n");
        
        // OutHeader = MoveTemp(OutHeaderText);
        // return true;
    }

    return true;
}
