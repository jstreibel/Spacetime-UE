#include "UnrealStructGenerator.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"

// Converts any snake_case, kebab-case, space separated, or camelCase string
// into PascalCase (e.g. "chat_message" → "ChatMessage", "sendMessage" → "SendMessage").
auto ToPascalCase = [](const FString& InString) -> FString
{
	FString Result;
	bool bCapNext = true;

	const int32 Len = InString.Len();
	for (int32 i = 0; i < Len; ++i)
	{
		const TCHAR Ch = InString[i];

		if (FChar::IsAlnum(Ch))
		{
			// If we see an uppercase in the middle of a word that follows a lowercase,
			// treat it as the start of a new PascalCase word.
			if (!bCapNext 
				&& FChar::IsUpper(Ch) 
				&& i > 0 
				&& FChar::IsLower(InString[i - 1]))
			{
				bCapNext = true;
			}

			if (bCapNext)
			{
				Result += FChar::ToUpper(Ch);
				bCapNext = false;
			}
			else
			{
				Result += FChar::ToLower(Ch);
			}
		}
		else
		{
			// Any non‐alphanumeric (underscore, dash, space, etc.) triggers a new word
			bCapNext = true;
		}
	}

	return Result;
};

bool FUnrealStructGenerator::Generate(
	const FTableSchema& Schema,
	FString& OutCode
)
{
	UE_LOG(LogTemp, Log,
		TEXT("[UnrealStructGenerator] Generating struct for table '%s'"),
		*Schema.TableName
	);

	// 1. Convert table name (e.g. "chat_message") to PascalCase ("ChatMessage")
	const FString BaseName = ToPascalCase(Schema.TableName);
	const FString StructName = FString::Printf(TEXT("F%s"), *BaseName);

	// 2. Begin USTRUCT block
	OutCode = FString::Printf(
		TEXT("USTRUCT(BlueprintType)\nstruct %s\n{\n    GENERATED_BODY()\n\n"),
		*StructName
	);

	// 3. Emit UPROPERTY and member for each field
	for (const FFieldInfo& Field : Schema.Fields)
	{
		OutCode += TEXT("    UPROPERTY(BlueprintReadWrite, Category=\"SpacetimeDB\")\n");
		OutCode += FString::Printf(
			TEXT("    %s %s;\n\n"),
			*Field.UEType,
			*Field.Name
		);
	}

	// 4. Close struct
	OutCode += TEXT("};\n");

	return true;
}
