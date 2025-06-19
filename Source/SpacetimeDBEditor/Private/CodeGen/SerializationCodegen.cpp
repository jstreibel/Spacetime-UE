#include "SerializationCodegen.h"

#include "Config.h"
#include "Parser/Common.h"
#include "Schema/RawModuleDefSchema.h"


namespace SpacetimeDB
{
	bool FSerializationCodegen::GenerateSerializationCode(
		const FRawModuleDef& ModuleDef,
		const FString& ModuleName,
		FString& OutSource,
		FString& OutHeader,
		FString& OutError)
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

		for (const auto &ExportedType : ModuleDef.ExportedTypes)
		{
			const auto Ref = ExportedType.TypeRef;
			const auto TypeName = ExportedType.Name.Name;

			const auto& TypeEntry = ModuleDef.Typespace.TypeEntries[Ref];

			if (auto SerializationInfo = FSerializationInfo{TypeName, OutHeader, OutSource};
				!SerializeAlgebraicType(TypeEntry, SerializationInfo, OutError))
			{
				return false;
			}
		}

		OutHeader += TEXT("}\n\n");

		OutSource += TEXT("}\n\n");
    
		return true;
	}

	bool FSerializationCodegen::SerializeAlgebraicType(
		const SpacetimeDB::FAlgebraicType& TypeEntry,
		FSerializationInfo &SerializationInfo,
		FString &OutError)
	{	
		switch (TypeEntry.Type)
		{
		case SpacetimeDB::EType::Product:
			GenerateProductSerializationCode(TypeEntry.Product, SerializationInfo);
			break;
		case SpacetimeDB::EType::Sum:
			GenerateSumSerializationCode(TypeEntry.Sum, SerializationInfo);
			break;

		default:
			SerializationInfo.OutHeader += TEXT("/* Unsupported type: " + SpacetimeDB::TypeToString(TypeEntry.Type) + " */\n");
			UE_LOG(LogTemp, Error, TEXT("[spacetime] Unsupported type: %s"), *SpacetimeDB::TypeToString(TypeEntry.Type));
			break;
		}

		return true;
	}

	void FSerializationCodegen::GenerateProductSerializationCode(
		const FProductType& Product,
		const FSerializationInfo& SerializationInfo)
	{
	
		auto &OutHeader = SerializationInfo.OutHeader;
		auto &OutSource = SerializationInfo.OutSource;

		const auto &TypeName = SerializationInfo.TypeName;

		const auto FunctionSignature = "void Serialize" + TypeName + "(\n"
			"    const F" + TypeName + "& SpacetimeProduct,\n"
			"    const FJsonWriterRef& Writer,\n"
			"    const TOptional<FString>& Key"; 

		OutHeader += FunctionSignature + "= {});\n\n";

		OutSource += FunctionSignature + ")\n"
		"{\n"
		"\n"
		"    ProductStart(Writer, Key);\n"
		"\n";

		for (const auto& Element : Product.Elements)
		{
			GenerateFunctionCall(Element, OutSource, EFieldKind::Product);
		}

		OutSource +=
		"\n"
		"\n"
		"    ProductEnd(Writer, Key);\n"
		"\n"
		"}"
		"\n"
		"\n";
	}

	void FSerializationCodegen::GenerateSumSerializationCode(
		const SpacetimeDB::FSumType& Sum,
		const FSerializationInfo& SerializationInfo)
	{
		auto &OutHeader = SerializationInfo.OutHeader;
		auto &OutSource = SerializationInfo.OutSource;

		const auto &TypeName = SerializationInfo.TypeName;
		const auto NormalizedTypeName = FCommon::ToPascalCase(TypeName);

		const auto FunctionSignature =
			"void Serialize" + TypeName + "(\n"
			"    const F" + TypeName + "& SpacetimeSum,\n"
			"    const FJsonWriterRef& Writer,\n"
			"    const TOptional<FString>& Key"; 

		OutHeader += FunctionSignature + "= {});\n\n";

		OutSource += FunctionSignature + ")\n"
		"{\n"
		"    // Sum:\n"
		"    SumStart(Writer, Key);\n"
		"\n"
		"    switch(SpacetimeSum.Tag)\n"
		"    {\n";

		{	
			uint8 OptionNumber = 0;

			const FString TagEnumName = "E" + NormalizedTypeName + "_Tags";
	
			for (const auto & [Tag, AlgebraicType] : Sum.Options)
			{
				// TODO: test consistency of anonymous Tagging
				const auto Name = Tag.IsSet() ? Tag.GetValue() : FString::FromInt(OptionNumber++);

				SpacetimeDB::FProductType::FField Field;
				Field.Name = Name;
				Field.AlgebraicType = AlgebraicType;
				OutSource +=
				"    case " + TagEnumName + "::" + Name + ":\n";

				const FString Key = TypeName;

				GenerateFunctionCall(Field, SerializationInfo.OutSource, EFieldKind::Sum, Key, 2);

				OutSource += "        break;\n";
			}
		}

		OutSource +=
		"    }\n"
		"    SumEnd(Writer, Key);\n"
		"}"
		"\n"
		"\n";
	
	}

	void FSerializationCodegen::GenerateFunctionCall(
		const SpacetimeDB::FProductType::FField& ProductField,
		FString& OutSource,
		const EFieldKind Kind,
		const TOptional<FString> &Tag,
		const int Indent)
	{
	
		if (!ProductField.Name.IsSet())
		{
			// TODO:
			// Ideally, we would use FCommon::MakeAnonymousDataMemberName() here, but it's not possible.
			// We can't handle usage of FCommon::MakeAnonymousDataMemberName() in case of anonymous fields,
			// since the name would differ from the name of the field generated in other parts of the code.
			// This is because the other parts would refer to the same type, but with different names generated
			// by the same FCommon::MakeAnonymousDataMemberName() call (this function produces a unique name
			// each time it's called'). Also TODO: make this explanation shorter.
		
			UE_LOG(LogTemp, Error, TEXT("[spacetime] Internal error: unhandled missing name for field in product type"));
			return;
		}

		const FString Name = ProductField.Name.GetValue();

		const auto NormalizedName = FCommon::ToPascalCase(Name);

		FString FunctionName = "Serialize";

		const auto AlgebraicKind = ProductField.AlgebraicType->Type;
		if (SpacetimeDB::IsBuiltIn(AlgebraicKind))
		{
			if (AlgebraicKind == SpacetimeDB::EType::Array
			  | AlgebraicKind == SpacetimeDB::EType::Map)
			{
				OutSource += "    // Unimplemented serialization of Map and Array Spacetime Builtin types.";
				OutSource += "    UE_LOG(LogTemp, Error, Skipping serialization of '" + Name + "'; "
							 "unimplemented serialization of Map and Array Spacetime Builtin types.\n";
				return;
			}

			FunctionName += "NumberOrString";
		}
		else
		{
			FunctionName += NormalizedName;
		}

		OutSource += "    \n";
		for (auto i = 0; i < Indent; ++i) OutSource += FSpacetimeConfig::TabString;
		OutSource += "// " + SpacetimeDB::TypeToString(AlgebraicKind) + ":\n";
		for (auto i = 0; i < Indent; ++i) OutSource += FSpacetimeConfig::TabString;
		const FString KindString = Kind == EFieldKind::Product ? "Product" : "Sum";
		OutSource += FunctionName + "(Spacetime" + KindString + "." + NormalizedName + ", Writer";

		if (Tag.IsSet()) OutSource += ", FString(\"" + Tag.GetValue() + "\")";

		OutSource += ");\n";
	}
}