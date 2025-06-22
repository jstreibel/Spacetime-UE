#include "CodeGen.h"

#include "ReducersCodegen.h"
#include "SerializationCodegen.h"
#include "TypesIRBuilder.h"
#include "Containers/UnrealString.h"
#include "Parser/Common.h"
#include "../Config.h"


namespace SpacetimeDB
{
    FString FCodeGen::ResolveAlgebraicTypeToUnrealCxx(const SpacetimeDB::FAlgebraicType& AlgebraicKind)
    {
        switch (AlgebraicKind.Type)
        {
            case EType::Product:  return "// Product placeholder";
            case EType::Sum:      return "// Sum placeholder";
            case EType::Ref:      return "// Ref placeholder";
            
            // case BuiltIn:
            case EType::Array:    return TEXT("// TArray placeholder");
            case EType::Map:      return TEXT("// TMap placeholder");

            default:   /* BuiltIn */
            return SpacetimeDB::MapBuiltinToUnreal(SpacetimeDB::TypeToString(AlgebraicKind.Type), MapToUnrealNativeRepresentation);
        }    
    }

    bool FCodeGen::GenerateTablesCode(
        const SpacetimeDB::FRawModuleDef& ModuleDef,
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
            if (ProductType.Type != SpacetimeDB::EType::Product)
            {
                OutError = TEXT("Table type is expected to be a SATS Product type.");
                return false;
            }

            FString StructName = TEXT("F") + FCommon::ToPascalCase(Table.Name) + TEXT("Row");
            OutHeaderText += TEXT("USTRUCT(BlueprintType)");
            OutHeaderText += TEXT("\nstruct ") + FSpacetimeConfig::ApiMacroString + " " + StructName + TEXT(" {\n\n"
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
                    *CxxTypeString, *FCommon::ToPascalCase(Name)
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

    bool FCodeGen::GenerateReducersCode(
        const FString& ModuleName,
        const FRawModuleDef& ModuleDef,
        const FTypesIR& ExportedTypesIR,
        FString& OutHeader,
        FString& OutSource,
        FString& OutError)
    {
        FReducersCodegen::EmitCode(ModuleName, ModuleDef, ExportedTypesIR, OutHeader, OutSource);

        return true;
    }

    bool FCodeGen::GenerateTypesSerializationCode(
        const FTypesIR& ExportedTypesIR,
        const FTypesIR& InlineTypesIR,
        const FString& ModuleName,
        FString& OutSource,
        FString& OutHeader,
        FString& OutError)
    {
        // return
        // FSerializationCodegen_deprecated::GenerateSerializationCode(
        //     ModuleDef, ModuleName, OutSource, OutHeader, OutError);

        FSerializationCodegen::EmitCode(ExportedTypesIR, InlineTypesIR, ModuleName, OutHeader, OutSource);

        return true;
    }

    void OutputTaggedUnion(
        const FTaggedUnion &TaggedUnion,
        const FString &ApiMacroString,
        FString &OutHeaderCode)
    {
        constexpr  bool bAdd_None_Tag = false;
        
        const auto TabString = FSpacetimeConfig::TabString;
    
        const auto TaggedUnionTagProperty = TabString +
            FString::Printf(TEXT("UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=\"SpacetimeDB|%ls\", meta=(SumTag=true))\n"),
                *TaggedUnion.SubCategory);
        const auto TaggedUnionOptionProperty = TabString + 
            FString::Printf(TEXT("UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=\"SpacetimeDB|%ls\", meta=(SumPayload=true))\n"),
                *TaggedUnion.SubCategory);
        const auto TagName = FString::Printf(TEXT("E%ls_Tags"), *TaggedUnion.Name);
    
        OutHeaderCode += FString::Printf(TEXT("UENUM(BlueprintType)\n"));
        OutHeaderCode += FString::Printf(TEXT("enum class %ls : uint8\n"), *TagName);
        OutHeaderCode += FString::Printf(TEXT("{\n"));
        if constexpr (bAdd_None_Tag)
            OutHeaderCode += TabString + FString::Printf(TEXT("None    UMETA(DisplayName=\"None\"),\n"));
        for (const auto& Option : TaggedUnion.Variants)
        {
            FString OptionName = Option.Type.RightChop(1);
            OutHeaderCode += TabString
            + FString::Printf(TEXT("%ls    UMETA(DisplayName=\"%ls\"),\n"),
                *OptionName, *OptionName);
        }
        OutHeaderCode += FString::Printf(TEXT("};\n"));
        OutHeaderCode += FString::Printf(TEXT("\n"));
        OutHeaderCode += FString::Printf(TEXT("USTRUCT(BlueprintType, Category=\"SpacetimeDB|%ls\")\n"),
            *TaggedUnion.SubCategory);
        OutHeaderCode += FString::Printf(TEXT("struct %ls F%ls : public FSpacetimeSum\n"), *ApiMacroString, *TaggedUnion.Name);
        OutHeaderCode += FString::Printf(TEXT("{\n"));
        OutHeaderCode += TabString + FString::Printf(TEXT("GENERATED_BODY()\n\n"));
        OutHeaderCode += TabString + FString::Printf(TEXT("// The current active payload\n"));
        OutHeaderCode += TaggedUnionTagProperty;
        
        if constexpr (bAdd_None_Tag)
            OutHeaderCode += TabString + FString::Printf(TEXT("%ls Tag = %ls::None;\n"), *TagName, *TagName);
        if constexpr (!bAdd_None_Tag)
        {
            if (TaggedUnion.Variants.Num() != 0)
            {
                OutHeaderCode += TabString
                + FString::Printf(TEXT("%ls Tag = %ls::%s;\n"),
                    *TagName, *TagName, *TaggedUnion.Variants[0].Type.RightChop(1));
            }
            else
            {
                OutHeaderCode += TabString + FString::Printf(TEXT("%ls Tag = %ls::%s;\n"), *TagName, *TagName, TEXT("<no tags available>"));
            }
        }
        

        for (const auto& Option : TaggedUnion.Variants)
        {
            OutHeaderCode += TabString + FString::Printf(TEXT("\n"));
            OutHeaderCode += TaggedUnionOptionProperty;
            OutHeaderCode += TabString + FString::Printf(TEXT("%ls %ls;\n"), *Option.Type, *Option.Name);
        }
    
        OutHeaderCode += TabString + FString::Printf(TEXT("\n"));
        OutHeaderCode += FString::Printf(TEXT("};\n\n\n"));
    }

    void OutputProduct(const FStruct& Struct, const FString& ApiMacro, FString &OutHeaderCode)
    {
        const auto TabString = FSpacetimeConfig::TabString;
    
        const auto & [
                ProductOrigin,
                Name,
                Attributes,
                bIsReflected,
                Specifiers,
                MetadataSpecifiers,
                Comment,
                bIsExportedType,
                TypespaceIndex]
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
        OutHeaderCode += TEXT("struct ") + ApiMacro + " " + Name + " : public FSpacetimeProduct\n"
        "{\n\n";

        if (bIsReflected)
        {
            OutHeaderCode += TabString + "GENERATED_BODY()\n\n";
        }

        for (const auto & [
            Origin,
            Name,
            Type,
            OriginalName,
            DefaultValue,
            Comment]
            : Attributes)
        {
            if (Comment.IsSet())
            {
                OutHeaderCode += TabString + "/* " + Comment.GetValue() + TEXT(" */\n");
            }
            
            if (bIsReflected)
            {
                OutHeaderCode += TabString + "UPROPERTY(BlueprintReadWrite)\n";
            }
            OutHeaderCode += TabString + Type + " " + Name;

            if (DefaultValue.IsSet())
            {
                OutHeaderCode += " = " + DefaultValue.GetValue();
            }

            OutHeaderCode += ";\n\n";
        }

        OutHeaderCode += "};\n\n\n";
    }

    bool RenderTypesIRToCode(const FTypesIR& TypesIR, FString &OutCode, FString &OutError, const bool TopoSort=false)
    {    
        // Cleanup
        OutCode = "";

        // TODO: add license
    
        if (TypesIR.bPragmaOnce) OutCode += TEXT("#pragma once\n\n");
    
        for (auto [Path, bIsLocal] : TypesIR.Includes)
        {
        
            OutCode += TEXT("#include ");

            if (bIsLocal)
            {
                OutCode += FString::Printf(TEXT("\"%s\""), *Path);
            }
            else
            {
                OutCode += FString::Printf(TEXT("<%s>"), *Path);
            }

            OutCode += TEXT("\n");
        
        }

        OutCode += TEXT("\n\n");

        TArray<FTypesIR::FHeaderElement> Elements;
        if (TopoSort)
        {
            Elements = TypesIR.GetTopoSortedElements();
        }
        else
        {
            Elements = TypesIR.GetAllElements();
        }
    
        for (const auto& Element : Elements)
        {
            if (Element.Type == FTypesIR::FHeaderElement::Struct)
            {
                const auto& ExportedStructs = TypesIR.GetStructs();
                const auto Index = Element.Index;
            
                if (Index >= ExportedStructs.Num())
                {
                    OutError = FString::Printf(TEXT(
                        "index (Index=%i, Num=%i) out of bounds for ExportedStructs element %s"),
                        Index, ExportedStructs.Num(), *Element.Name);

                    return false;
                }
                const auto& Struct = ExportedStructs[Index];
                OutputProduct(Struct, TypesIR.ApiMacro, OutCode);

                continue;
            }

            if (Element.Type == FTypesIR::FHeaderElement::TaggedUnion)
            {
                const auto& ExportedTaggedUnions = TypesIR.GetTaggedUnions();
                const auto Index = Element.Index;
            
                if (Index >= ExportedTaggedUnions.Num())
                {
                    OutError = FString::Printf(TEXT(
                        "index (Index=%i, Num=%i) out of bounds for ExportedTaggedUnions element %s"),
                        Index, ExportedTaggedUnions.Num(), *Element.Name);

                    return false;
                }
            
                const auto& TaggedUnion = ExportedTaggedUnions[Index];
                OutputTaggedUnion(TaggedUnion, TypesIR.ApiMacro, OutCode);

                continue;
            }

            UE_LOG(LogTemp, Error, TEXT("Unrecognized Element.Type for element named '%ls'"), *Element.Name);
        }

        return true;
    }

    bool FCodeGen::GenerateTypesCode(
        const FTypesIR& ExportedTypesIR,
        const FTypesIR& InlineTypesIR,
        FString& OutExportedTypesCode,
        FString& OutInlineTypesCode,
        FString& OutError)
    {
        UE_LOG(LogTemp, Log, TEXT("[SpacetimeDB] Successfully built header layout from IR"));

        if (!RenderTypesIRToCode(ExportedTypesIR, OutExportedTypesCode, OutError, true))
        {
            return false;
        }
    
        if (!RenderTypesIRToCode(InlineTypesIR, OutInlineTypesCode, OutError))
        {
            return false;
        }

        UE_LOG(LogTemp, Log, TEXT("[SpacetimeDB] Successfully rendered header layout to files"));
    
        return true;
    }
}