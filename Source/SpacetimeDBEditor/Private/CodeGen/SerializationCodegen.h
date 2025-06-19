#pragma once
#include "TypesHeadersIRBuilder.h"

class FSerializationCodegen
{
	
public:
	static bool GenerateSerializationCode(
		const SATS::FRawModuleDef& ModuleDef,
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
		const SATS::FAlgebraicType& TypeEntry,
		FSerializationInfo &SerializationInfo,		
		FString &OutError);

	static void GenerateProductSerializationCode(
		const SATS::FProductType& Product,
		const FSerializationInfo &SerializationInfo);

	static void GenerateSumSerializationCode(const SATS::FSumType& Sum, const FSerializationInfo& SerializationInfo);

	enum class EFieldKind
	{
		Product,
		Sum
	};
	static void GenerateFunctionCall(
		const SATS::FProductType::FField& ProductField,
		FString& OutSource,
		const EFieldKind Kind,
		const TOptional<FString> &Tag = {},
		const int Indent = 1);
};
