#pragma once
#include "Schema/RawModuleDefSchema.h"

class FCommon
{
public:
	static FString ArrayToString(const TArray<FString>& StringArray);
	
	static FString CreateUniqueName();
	
	// Unwraps an Option<String> JSON object ({ some: val } or { none: [] })
	static SATS::FOptionalString GetOptionalString(const TSharedPtr<FJsonObject>& OptionalString);

	// Converts any snake_case, kebab-case, space separated, or camelCase string
	// into PascalCase (e.g. "chat_message" → "ChatMessage", "sendMessage" → "SendMessage").
	static FString ToPascalCase(const FString& InString);
	
	static TSharedPtr<FJsonObject> ParseRequiredObject(
		const TSharedPtr<FJsonObject>& Parent,
		const FString& Field,
		FString& OutError);

	// Reads a required JSON array field, returning false + error if missing/not-array
	static bool ParseRequiredArray(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field,
							TArray<TSharedPtr<FJsonValue>>& OutArray, FString& OutError);
	
	static bool ParseRequiredString(
		const TSharedPtr<FJsonObject>& Parent,
		const FString& Field,
		FString &OutString,
		FString& OutError);

	// Unwraps an Option<String> JSON object ({ some: val } or { none: [] })
	static FString UnwrapOptionString(const TSharedPtr<FJsonObject>& OptionObj);
	
	static bool ParseNameAndAlgebraicType(
		const TSharedPtr<FJsonObject>& NameAndAlgTypePair,
		SATS::FOptionalString &OptionalName,
		SATS::FAlgebraicType& AlgebraicOut,
		FString& OutError);

	static bool ParseProduct(
		const TArray<TSharedPtr<FJsonValue>>& Elements, 
		SATS::FProductType& ProductOut,
		FString& OutError);

	static bool ParseSum(
		const TArray<TSharedPtr<FJsonValue>>& Variants,
		SATS::FSumType& SumOut,
		FString& OutError);

	static bool ParseBuiltin(
		const TSharedPtr<FJsonObject>& BuiltinObj,
		SATS::FBuiltinType &BuiltinOut,
		FString& OutError);

	/**
	 * 
	 * @param SatsJsonObject 
	 * @param AlgebraicOut 
	 * @param OutError 
	 * @return 
	 */
	static bool ResolveAlgebraicType(
		const TSharedPtr<FJsonObject>& SatsJsonObject,
		SATS::FAlgebraicType& AlgebraicOut,
		FString& OutError);

private:
	static bool ValidateAndGetSatsKindInTypespaceEntry(const TSharedPtr<FJsonObject>& TypeObj,
		SATS::EType &SatsKind, FString& OutError);
};
