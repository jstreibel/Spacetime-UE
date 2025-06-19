#pragma once
#include "Schema/RawModuleDefSchema.h"

class SPACETIMEDBCORE_API FCommon
{
public:
	// List of Unreal reserver class/struct/etc names
	// e.g. an user-implemented struct FPlayer conflicts with Unreal UPlayer class.
	static const TArray<FString> ReservedNames;

	static FString MakeStructName(const FString& InName, const FString& ModuleName);
	
	static FString ArrayToString(const TArray<FString>& StringArray);
	
	static FString MakeAnonymousDataMemberName();
	
	// Unwraps an Option<String> JSON object ({ some: val } or { none: [] })
	static SpacetimeDB::FOptionalString GetOptionalString(const TSharedPtr<FJsonObject>& OptionalString);

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
		SpacetimeDB::FOptionalString &OptionalName,
		SpacetimeDB::FAlgebraicType& AlgebraicOut,
		FString& OutError);

	static bool ParseProduct(
		const TArray<TSharedPtr<FJsonValue>>& Elements, 
		SpacetimeDB::FProductType& ProductOut,
		FString& OutError);

	static bool ParseSum(
		const TArray<TSharedPtr<FJsonValue>>& Variants,
		SpacetimeDB::FSumType& SumOut,
		FString& OutError);

	static bool ParseBuiltin(
		const TSharedPtr<FJsonObject>& BuiltinObj,
		SpacetimeDB::FBuiltinType &BuiltinOut,
		FString& OutError);

	/**
	 * This function validates and resolves the SATS Algebraic Type of a given JSON object
	 * @param SatsJsonObject the 'algebraic_type' JSON object  
	 * @param AlgebraicOut	 the fully parsed-out SATS algebraic type 	
	 * @param OutError		 error message, in case of error
	 * @return false, in case of error
	 */
	static bool ResolveAlgebraicType(
		const TSharedPtr<FJsonObject>& SatsJsonObject,
		SpacetimeDB::FAlgebraicType& AlgebraicOut,
		FString& OutError);

private:
	static bool ValidateAlgebraicTypeAndGetSatsKind(const TSharedPtr<FJsonObject>& TypeObj,
		SpacetimeDB::EType &SatsKind, FString& OutError);
};
