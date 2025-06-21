#include "ReducersCodegen.h"

#include "Config.h"
#include "Parser/Common.h"

namespace SpacetimeDB
{
	void FReducersCodegen::EmitCode(
		const FString& ModuleName,
		const FRawModuleDef& ModuleDef,
		const FTypesIR& TypesIR,
		FString& OutHeader,
		FString& OutSource)
	{
		const FString& HeaderName = FSpacetimeConfig::MakeReducerCodeFileName(ModuleName);
		const FString& ModuleNameNormalized = FCommon::ToPascalCase(ModuleName);

		OutHeader = TEXT(
			"#pragma once\n"
			"\n"
			"#include \"CoreMinimal.h\"\n"
			"#include \"Kismet/BlueprintFunctionLibrary.h\"\n"
			"#include \"" + FSpacetimeConfig::MakeExportedTypesCodeFileName(ModuleName) + ".h" + "\"\n"
			"#include \"SpacetimeDBConnectionSubsystem.h\"\n"
			"#include \"" + HeaderName + ".generated.h\"\n"
			"\n"
			"\n");

		OutSource += FString(
			"#include \"" + HeaderName + ".h\"\n"
			"\n"
			"#include \"SpacetimeDBConnectionSubsystem.h\"\n"
			"#include \"StdbGenerated/" + ModuleNameNormalized + "Serialization.stdbgen.h\"\n"
			"\n"
			"\n");
		
		for (const auto& Reducer : ModuleDef.Reducers)
		{
			const auto ParamsList = BuildParamsList(TypesIR, Reducer, true);
			EmitReducer(Reducer, ParamsList, ModuleNameNormalized,OutHeader, OutSource);
		}		
	}
	

	void FReducersCodegen::EmitReducer(
		const FReducerDef& Reducer,
		const TArray<FReducerParam>& ParamsList,
		const FString& ModuleNameNormalized,
		FString& OutHeader,
		FString& OutSource)
	{
		const FString ClassName = "UCall" + ModuleNameNormalized + Reducer.Name + "Reducer";
		const auto Signature = GenerateSignature(ParamsList, Reducer.Name, ClassName); 
			
		OutHeader += TEXT(
			"UCLASS()\n"
			"class " + FSpacetimeConfig::ApiMacroString + " " + ClassName + " : public USpacetimeAsyncReducerBase\n"
			"{\n"
			"\n"
			"    GENERATED_BODY()\n"
			"\n"
			"    UFUNCTION(BlueprintCallable, Category=\"SpacetimeDB|" + ModuleNameNormalized + "\", \n"
			"        meta=(BlueprintInternalUseOnly=\"true\", WorldContext = \"WorldContextObject\"))\n"
			"    static " + Signature.HeaderSignature + ";\n\n};\n\n\n");

		OutSource += Signature.SourceSignature + "\n"
		"{\n" 
		"    constexpr auto ReducerName = TEXT(\"" + Reducer.Name + "\");\n"
		"\n";
		
		if (ParamsList.Num() == 0)
		{
			OutSource +=
			"    const FString JsonPayload = \"[]\"; // No args, just an empty Json array.\n";
		}
		else
		{
			OutSource +=
			"    FString JsonPayload = \"[\"; // Payload is a Json array of serialized Algebraic types.\n";

			for (const auto& [Name, Type] : ParamsList)
			{
				const FString ChoppedType = Type.RightChop(1);
			
				OutSource += 
				"    {\n"
				"        FString JsonSerializedValue;\n"
				"        const auto WriterRef = FWriterFactory::Create(&JsonSerializedValue);\n"
				"        " + ModuleNameNormalized + "::Serialize" + ChoppedType + "(" + Name + ", WriterRef);\n"
				"        WriterRef->Close();\n"
				"        JsonPayload += JsonSerializedValue;\n"
				"        JsonPayload += \",\";\n"
				"    }\n";
			}

			OutSource += "    JsonPayload = JsonPayload.LeftChop(1); // Remove last comma.\n";
			OutSource += "    JsonPayload += \"]\";\n";
		}
		OutSource +=
		"\n"
		"    auto* Node = NewObject<" + ClassName + ">();\n"
		"    Node->Setup(WorldContextObject, ReducerName, JsonPayload);\n"
		"\n"
		"    return Node;\n"
		"}\n\n\n";
		
	}


	TArray<FReducersCodegen::FReducerParam> FReducersCodegen::BuildParamsList(
		const FTypesIR& ExportedTypesIR,
		const FReducerDef& Reducer,
		const bool bNormalizeNames)
	{
		const auto& ExportedElements = ExportedTypesIR.GetAllElements();
		const auto& ExportedProducts = ExportedTypesIR.GetStructs();
		const auto& ExportedSums = ExportedTypesIR.GetTaggedUnions();

		TArray<FReducerParam> ParamsList;
		for (const auto& [Name, Type] : Reducer.Params)
		{
			if (Type.Type != EType::Ref)
			{
				UE_LOG(LogTemp, Error, TEXT("[SpacetimeDB] Spacetime Unreal integration currently only support Ref parameters in Reducers"));
			}

			// TODO: IMPORTANT QUESTION: are all reducer parameters exported types?
			// If yes: are exported types mapped 1:1 to typespace?
			const auto RefIndex = Type.Ref.Index;
			const auto &Element = ExportedElements[RefIndex];

			FString TypeString;
			if (Element.Type == FTypesIR::FHeaderElement::Struct)
			{
				const auto& Struct = ExportedProducts[RefIndex];
				TypeString = Struct.Name;
			}
			else if (Element.Type == FTypesIR::FHeaderElement::TaggedUnion)
			{
				const auto& Sum = ExportedSums[RefIndex];
				TypeString = Sum.Name;
			}

			auto NameString = Name.IsSet() ? Name.GetValue() : "Value";
			if (bNormalizeNames)
			{
				NameString = FCommon::ToPascalCase(NameString);
			}

			ParamsList.Add({ NameString, TypeString });
		}

		return ParamsList;
	}
	

	FReducersCodegen::FReducerSignature FReducersCodegen::GenerateSignature(
		const TArray<FReducerParam>& Params,		
		const FString& ReducerName,
		const FString& ClassName)
	{
		FReducerSignature Signature;
		Signature.HeaderSignature = ClassName + "* " + ReducerName + "Reducer(\n        UObject* WorldContextObject";
		Signature.SourceSignature = ClassName + "* " + ClassName + "::" + ReducerName + "Reducer(UObject* WorldContextObject";

		for (const auto& [Name, Type] : Params)
		{
			Signature.HeaderSignature += FString::Printf(TEXT(",\n        const %s& %s"), *Type, *Name);
			Signature.SourceSignature += FString::Printf(TEXT(",\n        const %s& %s"), *Type, *Name);
		}

		Signature.HeaderSignature += FString(")");
		Signature.SourceSignature += FString(")");

		return Signature;
	}
}

