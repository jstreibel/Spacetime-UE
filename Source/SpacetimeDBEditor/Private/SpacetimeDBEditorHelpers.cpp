// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacetimeDBEditorHelpers.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "CodeGen/IFunctionGenerator.h"
#include "CodeGen/IStructGenerator.h"
#include "CodeGen/UnrealFunctionGenerator.h"
#include "CodeGen/UnrealStructGenerator.h"
#include "HAL/PlatformProcess.h"
#include "IO/CodeFileWriter.h"
#include "Parser/ModuleDefParser.h"
#include "Schema/SchemaModels.h"

static bool FetchRawData(
		const FString& DatabaseName,
		FString& OutRawModuleDef)
{
	// Build the CLI command and parameters
	const FString Executable = TEXT("spacetime");
	const FString Arguments  = FString::Printf(TEXT("describe --json %s"), *DatabaseName);

	UE_LOG(LogTemp, Log, TEXT("[spacetime] Running command: %s %s"), *Executable, *Arguments);  // :contentReference[oaicite:0]{index=0}

	int32 ReturnCode = -1;
	FString StdOut;
	FString StdErr;

	// Execute synchronously, capturing stdout and stderr
	const bool bLaunched = FPlatformProcess::ExecProcess(
		*Executable,
		*Arguments,
		&ReturnCode,
		&StdOut,
		&StdErr,
		nullptr,
		/*bShouldEndWithParentProcess=*/ false
	);  // :contentReference[oaicite:1]{index=1}

	if (!bLaunched)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[spacetime] Failed to launch SpacetimeDB CLI executable '%s'"),
			*Executable
		);
		return false;
	}

	// Check exit code
	if (ReturnCode != 0)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[spacetime] SpacetimeDB CLI exited with code %d"), ReturnCode
		);
		if (!StdErr.IsEmpty())
		{
			UE_LOG(LogTemp, Error,
				TEXT("[spacetime] SpacetimeDB CLI stderr: %s"), *StdErr
			);
		}
		return false;
	}

	// Warn if there was any stderr output despite success
	if (!StdErr.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[spacetime] SpacetimeDB CLI reported warnings: %s"), *StdErr
		);
	}

	// Log a bit of context for debugging
	UE_LOG(LogTemp, Log,
		TEXT("[spacetime] SpacetimeDB CLI returned %d bytes of JSON"), StdOut.Len()
	);

	// Return the raw JSON
	OutRawModuleDef = MoveTemp(StdOut);
	return true;
}

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

bool USpacetimeDBEditorHelpers::GenerateUSTRUCTsFromSchema(
	const FString& DatabaseName,
	FString& OutFullPath,
	FString& OutError)
{
	// 0. Fetch SpacetimeDB RawModuleDef
	FString RawModuleDef;
	if (!FetchRawData(DatabaseName, RawModuleDef))
	{
		OutError = TEXT("Failed to fetch raw module definition.");
		UE_LOG(LogTemp, Error, TEXT("%s"), *OutError);
		return false;
	}
	
	// 1. Parse raw JSON into schema models
    FModuleDefParser Parser;
    TArray<FTableSchema> Tables;
    TArray<FReducerSchema> Reducers;
    if (!Parser.Parse(RawModuleDef, Tables, Reducers, OutError))
    {
        UE_LOG(LogTemp, Error, TEXT("Schema parse failed: %s"), *OutError);
        return false;
    }

    // 2. Instantiate generators
    TUniquePtr<IStructGenerator> StructGen = MakeUnique<FUnrealStructGenerator>();
    TUniquePtr<IFunctionGenerator> FuncGen   = MakeUnique<FUnrealFunctionGenerator>();

    // 3. Prepare code writer
    FCodeFileWriter Writer;

    // 4. Determine output directory for generated files
    const FString OutputDir = FPaths::ProjectDir() / TEXT("Source/SpacetimeDB/Generated");
    if (!IFileManager::Get().MakeDirectory(*OutputDir, /*Tree=*/ true))
    {
        OutError = TEXT("Failed to create output directory.");
        UE_LOG(LogTemp, Error, TEXT("%s"), *OutError);
        return false;
    }

    // 5. Generate USTRUCT headers for tables
    for (const FTableSchema& Table : Tables)
    {
        FString StructCode;
        if (!StructGen->Generate(Table, StructCode))
        {
            OutError = FString::Printf(TEXT("Struct generation failed for table '%s'."), *Table.TableName);
            return false;
        }

        const FString StructFilename = FString::Printf(TEXT("F%s.generated.h"), *ToPascalCase(Table.TableName));
        const FString StructPath = OutputDir / StructFilename;
        if (!Writer.WriteFile(StructPath, StructCode, OutError))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to write struct file '%s': %s"), *StructPath, *OutError);
            return false;
        }
    }

    // 6. Generate Blueprint-callable reducer stubs
    // All reducers go into a single header
    FString FuncCode;
    for (const FReducerSchema& Reducer : Reducers)
    {
        FString Part;
        if (!FuncGen->Generate(Reducer, Part))
        {
            OutError = FString::Printf(TEXT("Function generation failed for reducer '%s'."), *Reducer.ReducerName);
            return false;
        }
        FuncCode.Append(Part);
    }
    const FString FuncHeader = OutputDir / TEXT("SpacetimeDBReducers.generated.h");
    if (!Writer.WriteFile(FuncHeader, FuncCode, OutError))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to write reducer header: %s"), *OutError);
        return false;
    }

    // 7. Return success
    OutFullPath = OutputDir;
    UE_LOG(LogTemp, Log, TEXT("Generated code in directory: %s"), *OutputDir);
    return true;
}
