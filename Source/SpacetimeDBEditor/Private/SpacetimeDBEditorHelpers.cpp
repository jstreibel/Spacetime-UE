// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacetimeDBEditorHelpers.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "CodeGen/FSpacetimeDBCodegen.h"
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
	// 0. Fetch raw JSON schema
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Fetching RawModuleDef for '%s'"), *DatabaseName);
    FString RawModuleDefString;
    if (!FetchRawData(DatabaseName, RawModuleDefString))
    {
        OutError = TEXT("Failed to fetch raw module definition.");
        UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
        return false;
    }

    // 1. Parse into SATS model
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Parsing RawModuleDef JSON"));
    SATS::FRawModuleDef RawModule;
	if (FModuleDefParser Parser; !Parser.Parse(RawModuleDefString, RawModule, OutError))
    {
        UE_LOG(LogTemp, Error, TEXT("[spacetime] Schema parse failed: %s"), *OutError);
        return false;
    }

    // 2. Instantiate code generator
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Initializing code generator"));
    TUniquePtr<FSpacetimeDBCodeGen> CodeGen = MakeUnique<FSpacetimeDBCodeGen>();

    // 3. Prepare file writer
    FCodeFileWriter Writer;

    // 4. Ensure output directory exists
    const FString OutputDir = FPaths::ProjectDir() / TEXT("Source/SpacetimeDB/Generated");
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Creating output directory '%s'"), *OutputDir);
    if (!IFileManager::Get().MakeDirectory(*OutputDir, /*Tree=*/ true))
    {
        OutError = TEXT("Failed to create output directory.");
        UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
        return false;
    }

    // 5. Generate table structs header
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating table USTRUCTs"));
    FString TablesHeader;
    if (!CodeGen->GenerateTableStructs(RawModule, TablesHeader))
    {
        OutError = TEXT("Table struct generation failed.");
        UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
        return false;
    }
    const FString TablesHeaderName = FString::Printf(TEXT("F%sTables.generated.h"), *DatabaseName);
    const FString TablesHeaderPath = OutputDir / TablesHeaderName;
    if (!Writer.WriteFile(TablesHeaderPath, TablesHeader, OutError))
    {
        UE_LOG(LogTemp, Error, TEXT("[spacetime] Failed to write tables header '%s': %s"), *TablesHeaderPath, *OutError);
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Wrote %s"), *TablesHeaderPath);

    // 6. Generate reducer functions (header + source)
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating reducer Blueprint nodes"));
    FString ReducersHeader, ReducersSource;
    if (!CodeGen->GenerateReducerFunctions(RawModule, ReducersHeader, ReducersSource))
    {
        OutError = TEXT("Reducer function generation failed.");
        UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
        return false;
    }
    const FString ReducersHeaderName = FString::Printf(TEXT("F%sReducers.generated.h"), *DatabaseName);
    const FString ReducersHeaderPath = OutputDir / ReducersHeaderName;
    if (!Writer.WriteFile(ReducersHeaderPath, ReducersHeader, OutError))
    {
        UE_LOG(LogTemp, Error, TEXT("[spacetime] Failed to write reducers header '%s': %s"), *ReducersHeaderPath, *OutError);
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Wrote %s"), *ReducersHeaderPath);

    const FString ReducersSourceName = FString::Printf(TEXT("F%sReducers.generated.cpp"), *DatabaseName);
    const FString ReducersSourcePath = OutputDir / ReducersSourceName;
    if (!Writer.WriteFile(ReducersSourcePath, ReducersSource, OutError))
    {
        UE_LOG(LogTemp, Error, TEXT("[spacetime] Failed to write reducers source '%s': %s"), *ReducersSourcePath, *OutError);
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Wrote %s"), *ReducersSourcePath);

    // 7. Success
    OutFullPath = OutputDir;
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Code generation completed for database '%s'"), *DatabaseName);
    return true;
    
}
