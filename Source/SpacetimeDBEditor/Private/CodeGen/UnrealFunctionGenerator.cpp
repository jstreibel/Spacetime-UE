#include "UnrealFunctionGenerator.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"

bool FUnrealFunctionGenerator::Generate(
	const FReducerSchema& Schema,
	FString& OutCode
)
{
	// Log generation start
	UE_LOG(LogTemp, Log,
		TEXT("[UnrealFunctionGenerator] Generating function for reducer '%s'"),
		*Schema.ReducerName
	);

	// 1. Convert reducer name (e.g. "send_message") to PascalCase ("SendMessage")
	auto ToPascalCase = [](const FString& InString) {
		TArray<FString> Parts;
		InString.ParseIntoArray(Parts, TEXT("_"), true);
		FString Result;
		for (const FString& Part : Parts)
		{
			if (!Part.IsEmpty())
			{
				Result += Part.Left(1).ToUpper() + Part.Mid(1);
			}
		}
		return Result;
	};
	const FString FuncName = ToPascalCase(Schema.ReducerName);

	// 2. Build parameter declarations array
	TArray<FString> ParamDecls;
	for (const FFieldInfo& Param : Schema.Params)
	{
		// Use UEType (e.g. "int32", "FString") and original field name
		ParamDecls.Add(FString::Printf(TEXT("%s %s"), *Param.UEType, *Param.Name));
	}
	const FString ParamList = FString::Join(ParamDecls, TEXT(", "));

	// 3. Emit UFUNCTION stub
	OutCode = FString::Printf(
		TEXT("    UFUNCTION(BlueprintCallable, Category=\"SpacetimeDB\")\n")
		TEXT("    static void %s(%s);\n\n"),
		*FuncName,
		*ParamList
	);

	return true;
}

