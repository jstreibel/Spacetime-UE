#include "SpacetimeDBCodegen.h"

#include "TypespaceStructGen.h"
#include "Containers/UnrealString.h"
#include "Parser/Common.h"

#define STDB_USE_DEPR_METHOD false

const FString ApiMacroString = TEXT("SPACETIMEDBRUNTIME_API");
const FString TabString = TEXT("    ");

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
            // If we see an uppercase letter in the middle of a word that follows a lowercase letter,
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
    default:   /* BuiltIn */    return SATS::MapBuiltinToUnreal(SATS::TypeToString(AlgebraicKind.Tag), false);
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
        if (ProductType.Tag != SATS::EType::Product)
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
    const FString& ModuleName,
    const SATS::FRawModuleDef& ModuleDef,
    const FString& HeaderName,
    FString& OutHeader,
    FString& OutSource,
    FString& OutError
)
{
    TArray<SATS::FExportedType> SortedRefs = ModuleDef.Types;
    Algo::Sort(SortedRefs, [](const SATS::FExportedType& EntryA, const  SATS::FExportedType& EntryB)
    {
        return EntryA.TypeRef < EntryB.TypeRef;
    });
    
    
    const FString ClassName = "U" + ModuleName +  "Reducers";
    
    // Header
    FString HeaderText;
    HeaderText += TEXT("#pragma once\n\n"
                "#include \"Kismet/BlueprintFunctionLibrary.h\"\n"
                "#include \"CoreMinimal.h\"\n"
                "#include \"" + ModuleName + "Typespace.h" + "\"\n"
                "#include \"" + HeaderName + ".generated.h" + "\"\n\n\n");
    HeaderText += TEXT("UCLASS()\n"
                "class " + ApiMacroString + " " + ClassName + " : public UBlueprintFunctionLibrary {\n\n"
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

            FString UEType;
            
            if (IsBuiltinWithNativeRepresentation(AlgebraicType.Tag))
            {
                UEType = ResolveAlgebraicTypeToUnrealCxx(AlgebraicType);
            }
            else if (AlgebraicType.Tag == SATS::EType::Ref)
            {
                const auto Index = AlgebraicType.Ref.Index;
                const auto TypeName = SortedRefs[Index].Name.Name;

                UEType = FCommon::MakeStructName(TypeName, ModuleName);
            }            
            else
            {
                UE_LOG(LogTemp, Error, TEXT("SpacetimeDB Reducer Unreal codegen currently supports only 'BuiltIn' and 'Ref' SATS-JSON Types"));
            }
            
            const FString ParamArgString = FString::Printf(TEXT("const %s& %s"), *UEType, *ArgName);
            
            Params.Add(ParamArgString);
        }

        FString Prefix = TEXT("    UFUNCTION(BlueprintCallable, Category=\"SpacetimeDB|" + ModuleName + "\")"); 
        FString Sig = Prefix + FString::Printf(
            TEXT("\n    static void %s(%s);\n\n"),
            *ToPascalCase(ReducerDef.Name), *FString::Join(Params, TEXT(", "))
        );
        HeaderText += Sig;

        // Implementation stub
        FString Suffix = FString::Printf(
            TEXT("::%s(%s)\n{\n    // TODO: call SpacetimeDB client reducer '%s'\n}\n\n"),
            *ToPascalCase(ReducerDef.Name), *FString::Join(Params, TEXT(", ")),
            *ReducerDef.Name
        );
        FString Impl = "void " + ClassName + Suffix;
        Src += "\n" + Impl;
    }
    HeaderText += TEXT("};\n");

    OutHeader = MoveTemp(HeaderText);
    OutSource = MoveTemp(Src);
    return true;
}

void GOutputTaggedUnion(const FTaggedUnion &TaggedUnion, FString &OutHeaderCode)
{
    const auto TaggedUnionOptionProperty = TabString +
        FString::Printf(TEXT("UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=\"SpacetimeDB|%ls\")\n"),
            *TaggedUnion.SubCategory);
    const auto TagName = FString::Printf(TEXT("E%ls_Tags"), *TaggedUnion.BaseName);
    
    OutHeaderCode += FString::Printf(TEXT("UENUM(BlueprintType)\n"));
    OutHeaderCode += FString::Printf(TEXT("enum class %ls : uint8\n"), *TagName);
    OutHeaderCode += FString::Printf(TEXT("{\n"));
    OutHeaderCode += TabString + FString::Printf(TEXT("None    UMETA(DisplayName=\"None\"),\n"));
    for (const auto& Option : TaggedUnion.OptionTags)
    {
        FString OptionName = Option.RightChop(1);
        OutHeaderCode += TabString + FString::Printf(TEXT("%ls    UMETA(DisplayName=\"%ls\"),\n"), *OptionName, *OptionName);
    }
    OutHeaderCode += FString::Printf(TEXT("};\n"));
    OutHeaderCode += FString::Printf(TEXT("\n"));
    OutHeaderCode += FString::Printf(TEXT("USTRUCT(BlueprintType, Category=\"SpacetimeDB|%ls\")\n"), *TaggedUnion.SubCategory);
    OutHeaderCode += FString::Printf(TEXT("struct F%ls\n"), *TaggedUnion.BaseName);
    OutHeaderCode += FString::Printf(TEXT("{\n"));
    OutHeaderCode += TabString + FString::Printf(TEXT("GENERATED_BODY()\n"));
    OutHeaderCode += TabString + FString::Printf(TEXT("\n"));
    OutHeaderCode += TabString + FString::Printf(TEXT("// Active payload\n"));
    OutHeaderCode += TaggedUnionOptionProperty;
    OutHeaderCode += TabString + FString::Printf(TEXT("%ls Tag = %ls::None;\n"), *TagName, *TagName);

    for (const auto& Option : TaggedUnion.Variants)
    {
        OutHeaderCode += TabString + FString::Printf(TEXT("\n"));
        OutHeaderCode += TaggedUnionOptionProperty;
        OutHeaderCode += TabString + FString::Printf(TEXT("%ls %ls;\n"), *Option.Type, *Option.Name);
    }
    
    OutHeaderCode += TabString + FString::Printf(TEXT("\n"));
    OutHeaderCode += FString::Printf(TEXT("};\n\n\n"));
}

void GOutputStruct(const FStruct& Struct, const FString& ApiMacro, FString &OutHeaderCode)
{
    const auto & [
            Name,
            Attributes,
            bIsReflected,
            Specifiers,
            MetadataSpecifiers,
            Comment]
    = Struct;
    
    if (Comment.IsSet())
    {
        OutHeaderCode += "/* " + Comment.GetValue() + " */\n";
    }
        
    if (bIsReflected)
    {
        OutHeaderCode += TEXT("USTRUCT(");
            
        for (const auto &Specifier : Specifiers)
        {
            OutHeaderCode += Specifier + TEXT(", ");
        }

        for (const auto &MetaSpecifiers : MetadataSpecifiers)
        {
            OutHeaderCode += MetaSpecifiers.Key + "=" + MetaSpecifiers.Value;
        }

        OutHeaderCode.RemoveFromEnd(", ");

        OutHeaderCode += ")\n";
    }
    OutHeaderCode += TEXT("struct ") + ApiMacro + " " + Name + " {\n\n";

    if (bIsReflected)
    {
        OutHeaderCode += TabString + "GENERATED_BODY();\n\n";
    }

    for (const auto &Attribute : Attributes)
    {
        if (Attribute.Comment.IsSet())
        {
            OutHeaderCode += TabString + "/* " + Attribute.Comment.GetValue() + TEXT(" */\n");
        }
            
        if (bIsReflected)
        {
            OutHeaderCode += TabString + "UPROPERTY(BlueprintReadWrite)\n";
        }
        OutHeaderCode += TabString + Attribute.Type + " " + Attribute.Name + ";" + "\n\n";
    }

    OutHeaderCode += "};\n\n\n";
}

bool FSpacetimeDBCodeGen::GenerateTypespaceStructs(
    const SATS::FRawModuleDef& ModuleDef,
    const FString& ModuleName,
    const FString& HeaderName,
    FString& OutHeaderCode,
    FString& OutError)
{
    FHeader Header;
    
    if (!FTypespaceStructGen::BuildExportedTypesHeader(
        ModuleName,
        HeaderName,
        ModuleDef.Typespace,
        ModuleDef.Types,
        Header,
        OutError))
    {
        OutError = TEXT("Failed to generate header data from typespace: ") + OutError;
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("[spacetime] Successfully built header layout from IR"));

    if (Header.bPragmaOnce) OutHeaderCode += TEXT("#pragma once\n\n");
    
    for (auto [Path, bIsLocal] : Header.Includes)
    {
        
        OutHeaderCode += TEXT("#include ");

        if (bIsLocal)
        {
            OutHeaderCode += FString::Printf(TEXT("\"%s\""), *Path);
        }
        else
        {
            OutHeaderCode += FString::Printf(TEXT("<%s>"), *Path);
        }

        OutHeaderCode += TEXT("\n");
        
    }

    OutHeaderCode += TEXT("\n\n");

    #if STDB_USE_DEPR_METHOD == true
    {
        for (const auto& TaggedUnion : Header.TaggedUnions)
        {
            GOutputTaggedUnion(TaggedUnion, OutHeaderCode);
        }

        for (const auto & Struct : Header.Structs)
        {
            GOutputStruct(Struct, Header.ApiMacro, OutHeaderCode);
        }
    }
    #else
    for (const auto& Element : Header.HeaderElements)
    {
        if (Element.Type == FHeader::FHeaderElement::Struct)
        {
            const auto& Struct = Header.Structs[Element.Index];
            GOutputStruct(Struct, Header.ApiMacro, OutHeaderCode);

            continue;
        }

        if (Element.Type == FHeader::FHeaderElement::TaggedUnion)
        {
            const auto& TaggedUnion = Header.TaggedUnions[Element.Index];
            GOutputTaggedUnion(TaggedUnion, OutHeaderCode);

            continue;
        }

        UE_LOG(LogTemp, Error, TEXT("Unrecognized Element.Type for element named '%ls'"), *Element.Name);
    }
    #endif    

    return true;
}
