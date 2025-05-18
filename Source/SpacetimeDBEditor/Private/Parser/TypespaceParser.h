#pragma once
#include "Schema/RawModuleDefSchema.h"

class FTypespaceParser
{
public:
	static bool ParseNameAndAlgebraicType(
		const TSharedPtr<FJsonObject>& NameAlgTypePair,
		FString &NameString,
		SATS::FAlgebraicKind& AlgebraicOut,
		FString& OutError);
	
	static bool ParseTypespace(
		const TSharedPtr<FJsonObject>& RawModuleDefJson,
		SATS::FTypespace& TypespaceOutput,
		FString& OutError);

	static bool ParseTypespaceProduct(
		const TArray<TSharedPtr<FJsonValue>>& Elements, 
        SATS::FProductKind& ProductOut,
        FString& OutError);

    static bool ParseTypespaceSum(
    	const TArray<TSharedPtr<FJsonValue>>& Variants,
        SATS::FSumKind& SumOut,
        FString& OutError);

    static bool ParseTypespaceBuiltin(
    	const TSharedPtr<FJsonObject>& BuiltinObj,
        SATS::FBuiltinKind &BuiltinOut,
        FString& OutError);

    
    static bool ParseTypespaceAlgebraicType(
    	const TSharedPtr<FJsonObject>& SatsJsonObject,
        SATS::FAlgebraicKind& AlgebraicOut,
        FString& OutError);
};
