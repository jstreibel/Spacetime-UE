#pragma once
#include "TypesIRBuilder.h"


namespace SpacetimeDB
{
	class FSerializationCodegen_deprecated
	{
	
	public:
		static bool GenerateSerializationCode(
			const FRawModuleDef& ModuleDef,
			const FString& ModuleName,
			FString& OutSource,
			FString& OutHeader,
			FString& OutError);

	private:
		struct FSerializationInfo
		{
			const FString &TypeName;
			FString &OutHeader;
			FString &OutSource;		
		};
	
		static bool SerializeAlgebraicType(
			const FAlgebraicType& TypeEntry,
			const FSerializationInfo &SerializationInfo,		
			FString &OutError);

		static void GenerateProductSerializationCode(
			const FProductType& Product,
			const FSerializationInfo &SerializationInfo);

		static void GenerateSumSerializationCode(const SpacetimeDB::FSumType& Sum, const FSerializationInfo& SerializationInfo);

		enum class EFieldKind
		{
			Product,
			Sum
		};
		static void GenerateFunctionCall(
			const FProductType::FField& ProductField,
			FString& OutSource,
			const EFieldKind Kind,
			const TOptional<FString> &Tag = {},
			const int Indent = 1);
	};
}