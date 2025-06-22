#include "SerializationCodegen.h"

#include "Config.h"
#include "Parser/Common.h"


void SpacetimeDB::FSerializationCodegen::EmitCode(
	const FTypesIR& ExportedTypesIR,
	const FTypesIR& InlineTypesIR,
	const FString& ModuleName,
	FString& OutHeader,
	FString& OutSource)
{
	const FString ModuleNamePascalCase = FCommon::ToPascalCase(ModuleName);
    
	OutHeader = TEXT("#pragma once\n"
					 "\n"
					 "#include \"CoreMinimal.h\"\n"
					 "#include \"SpacetimeRuntimeSDK.h\"\n"
					 "#include \"" + ModuleNamePascalCase + "ExportedTypes.stdbgen.h\"\n"
					 "\n"
					 "\n"
					 "namespace " + ModuleNamePascalCase + " {\n\n");

	OutSource = TEXT("#include \"" + ModuleNamePascalCase + "Serialization.stdbgen.h\"\n"
					 "\n"
					 "\n"
					 "namespace " + ModuleNamePascalCase + " {\n\n");

	EmitTypesIRCode(InlineTypesIR, OutHeader, OutSource);
	EmitTypesIRCode(ExportedTypesIR, OutHeader, OutSource);	

	OutHeader += TEXT("}\n\n");
	OutSource += TEXT("}\n\n");
}

void SpacetimeDB::FSerializationCodegen::EmitTypesIRCode(
	const FTypesIR& TypesIR,
	FString& OutHeader,
	FString& OutSource)
{
	const auto& Products = TypesIR.GetStructs();
	const auto& Sums = TypesIR.GetTaggedUnions();
	
	for (const auto& TypeReference : TypesIR.GetAllElements())
	{
		if (TypeReference.Type == FTypesIR::FHeaderElement::TaggedUnion)
		{
			const auto& Sum = Sums[TypeReference.Index];
			GenerateSumSerializationCode(Sum, OutHeader, OutSource);
		}
		
		else if (TypeReference.Type == FTypesIR::FHeaderElement::Struct)
		{
			const auto& Struct = Products[TypeReference.Index];
			GenerateProductSerializationCode(Struct, OutHeader, OutSource);
		}
	}
}


void SpacetimeDB::FSerializationCodegen::GenerateSumSerializationCode(
	const FTaggedUnion& Sum,
	FString& OutHeader,
	FString& OutSource)
{
	const auto &TypeName = Sum.Name;
	const auto &TypeNameClear = TypeName.RightChop(1); // remove the leading 'F'
	
	const auto FunctionSignature =
			"void Serialize" + TypeNameClear + "(\n"
			"    const " + TypeName + "& Value,\n"
			"    const SpacetimeDB::FJsonWriterRef& Writer,\n"
			"    const TOptional<FString>& Key"; 

	OutHeader += FunctionSignature + "= {});\n\n";

	OutSource += FunctionSignature + ")\n"
	"{\n"
	"    // Sum:\n"
	"    SpacetimeDB::SumStart(Writer, Key);\n"
	"\n"
	"    switch(Value.Tag)\n"
	"    {\n";

	{
		const FString TagEnumName = "E" + TypeNameClear + "_Tags";
		
		for (const auto& Variant: Sum.Variants)
		{
			// TODO: test consistency of anonymous Tagging
			// const auto Name = Tag.IsSet() ? Tag.GetValue() : FString::FromInt(OptionNumber++);

			const auto& TagValue = Variant.Name;
			const FString Key = Variant.Name;
			
			OutSource += "    case " + TagEnumName + "::" + TagValue + ":\n";
			
			EmitFunctionCall(Variant, OutSource, Key, 2);

			OutSource += "        break;\n";
		}
	}

	OutSource +=
	"    }\n"
	"    SpacetimeDB::SumEnd(Writer, Key);\n"
	"}"
	"\n"
	"\n";
}


void SpacetimeDB::FSerializationCodegen::GenerateProductSerializationCode(
	const FStruct& Struct,
	FString& OutHeader,
	FString& OutSource)
{
	const auto &TypeName = Struct.Name;
	const auto TypeNameClear = TypeName.RightChop(1); // remove the leading 'F'

	const auto FunctionSignature = "void Serialize" + TypeNameClear + "(\n"
		"    const " + TypeName + "& Value,\n"
		"    const SpacetimeDB::FJsonWriterRef& Writer,\n"
		"    const TOptional<FString>& Key"; 

	OutHeader += FunctionSignature + "= {});\n\n";

	OutSource += FunctionSignature + ")\n"
	"{\n"
	"\n"
	"    SpacetimeDB::ProductStart(Writer, Key);\n";

	for (const auto& Element : Struct.DataMembers)
	{
		EmitFunctionCall(Element, OutSource);
	}

	OutSource +=
	"\n"
	"    SpacetimeDB::ProductEnd(Writer, Key);\n"
	"\n"
	"}"
	"\n"
	"\n";
}

void SpacetimeDB::FSerializationCodegen::EmitFunctionCall(
	const FDataMember& ToType,
	FString& OutSource,
	TOptional<FString> Tag,
	const int Indent)
{
	const auto& Name = ToType.Name;
	const auto& TypeString = ToType.Type;
	FString FunctionName = "Serialize";
	FString Namespace = "";
	
	const auto AlgebraicKind = ToType.Origin->Type;
	if (IsBuiltIn(AlgebraicKind))
	{
		if (AlgebraicKind == EType::Array
		  | AlgebraicKind == EType::Map)
		{
			OutSource += "    // Unimplemented serialization of Map and Array Spacetime Builtin types.";
			OutSource += "    UE_LOG(LogTemp, Error, Skipping codegen of '" + TypeString + "' function call; "
						 "unimplemented serialization of Map and Array Spacetime Builtin types.\n";
			return;
		}

		if (HasNativeUnrealRepresentation(AlgebraicKind))
		{
			FunctionName += "NumberOrString";
			Namespace = "SpacetimeDB::";
		}
		else if (IsBuiltInAdded(AlgebraicKind))
		{
			FunctionName += "BuiltIn_Added";
			Namespace = "SpacetimeDB::";
		}
		else
		{
			FunctionName += "<unknown BuiltIn Kind>";
		}
	}
	else
	{
		FunctionName += TypeString.RightChop(1); // Remove the leading 'F'
	}

	OutSource += "    \n";
	for (auto i = 0; i < Indent; ++i) OutSource += FSpacetimeConfig::TabString;
	OutSource += "// " + TypeToString(AlgebraicKind) + ":\n";
	for (auto i = 0; i < Indent; ++i) OutSource += FSpacetimeConfig::TabString;
	OutSource += Namespace + FunctionName + "(Value" + "." + Name + ", Writer";

	if (Tag.IsSet()) OutSource += ", FString(\"" + Tag.GetValue() + "\")";

	OutSource += ");\n";
}