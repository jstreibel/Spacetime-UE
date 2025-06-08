// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacetimeDBEditorHelpers.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "CodeGen/FSpacetimeDBCodegen.h"
#include "HAL/PlatformProcess.h"
#include "IO/CodeFileWriter.h"
#include "Parser/ModuleDefParser.h"
#include "Schema/SchemaModels.h"

#include <SpacetimeDBEditorHelpers.h>

bool RawModuleDefFromCli(
	const FString &DatabaseName,
	FString &Output);

bool RawModuleDefFromHttp(
	const FString& ServerURL,
	const FString& DatabaseName,
	FString& OutRawModuleDef);

static bool FetchRawData(
	const FString& ServerURL,
	const FString& DatabaseName,
	FString& OutRawModuleDef)
{	
	//return RawModuleDefFromCli(DatabaseName, OutRawModuleDef);
	return RawModuleDefFromHttp(ServerURL, DatabaseName, OutRawModuleDef);
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

bool USpacetimeDBEditorHelpers::GenerateCxxUnrealCodeFromSpacetimeDB(
	const FString& ServerURL,
	const FString& DatabaseName,
	FString& OutFullPath,
	FString& OutError)
{
	const FString DatabaseNamePascal = ToPascalCase(DatabaseName);
	

	// 0. Fetch raw JSON schema
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Fetching RawModuleDef for '%s'"), *DatabaseName);
    FString RawModuleDefString;
    if (!FetchRawData(ServerURL, DatabaseName, RawModuleDefString))
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
        UE_LOG(LogTemp, Error, TEXT("[spacetime] RawModuleDef parse failed: %s"), *OutError);
        return false;
    }


	// 2. Instantiate code generator
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Initializing code generator"));
    TUniquePtr<FSpacetimeDBCodeGen> CodeGen = MakeUnique<FSpacetimeDBCodeGen>();


	// 3. Prepare file writer
    FCodeFileWriter Writer;


	// 4. Ensure output directory exists
    // const FString OutputDir = FPaths::ProjectDir() / TEXT("Source/SpacetimeDB/Generated");
	const FString OutputDir = FPaths::ProjectDir() / TEXT("Plugins/SpacetimeDB/Source/SpacetimeDB/<Public&Private>/Generated");
	const FString HeaderOutputDir = FPaths::ProjectDir() / TEXT("Plugins/SpacetimeDB/Source/SpacetimeDB/Public/Generated");
	const FString SourceOutputDir = FPaths::ProjectDir() / TEXT("Plugins/SpacetimeDB/Source/SpacetimeDB/Private/Generated");
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Creating output directories '%s' and '%s'"), *HeaderOutputDir, *SourceOutputDir);
    if (!IFileManager::Get().MakeDirectory(*HeaderOutputDir, /*Tree=*/ true) | !IFileManager::Get().MakeDirectory(*SourceOutputDir, /*Tree=*/ true))
    {
        OutError = TEXT("Failed to create output directory.");
        UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
        return false;
    }

	
    // 5. Generate table structs header
	if (0)
	{
		UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating table USTRUCTs"));
		FString TablesHeader;
		FString BaseTablesHeaderName = FString::Printf(TEXT("%sTables"), *DatabaseNamePascal);
		if (!CodeGen->GenerateTableStructs(RawModule, BaseTablesHeaderName, TablesHeader, OutError))
		{
			OutError = TEXT("Table struct generation failed: ") + OutError;
			UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
			return false;
		}
		const FString TablesHeaderName = FString::Printf(TEXT("%sTables.h"), *DatabaseNamePascal);
		const FString TablesHeaderPath = HeaderOutputDir / TablesHeaderName;
		if (!Writer.WriteFile(TablesHeaderPath, TablesHeader, OutError))
		{
			UE_LOG(LogTemp, Error, TEXT("[spacetime] Failed to write tables header '%s': %s"), *TablesHeaderPath, *OutError);
			return false;
		}
		UE_LOG(LogTemp, Log, TEXT("[spacetime] Wrote %s"), *TablesHeaderPath);
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[spacetime] Table code generation not implemented"));
	}

	
    // 6. Generate reducer functions (header + source)
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating reducer Blueprint nodes"));
    FString ReducersHeader, ReducersSource;
	FString BaseReducersHeaderName = FString::Printf(TEXT("%sReducers"), *DatabaseNamePascal);
    if (!CodeGen->GenerateReducerFunctions(
    	RawModule, BaseReducersHeaderName,
    	ReducersHeader,
    	ReducersSource,
    	OutError))
    {
        OutError = TEXT("Reducer function generation failed: ") + OutError;
        UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
        return false;
    }
    const FString ReducersHeaderName = FString::Printf(TEXT("%sReducers.h"), *DatabaseNamePascal);
    const FString ReducersHeaderPath = HeaderOutputDir / ReducersHeaderName;
    if (!Writer.WriteFile(ReducersHeaderPath, ReducersHeader, OutError))
    {
        UE_LOG(LogTemp, Error, TEXT("[spacetime] Failed to write reducers header '%s': %s"), *ReducersHeaderPath, *OutError);
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Wrote %s"), *ReducersHeaderPath);

    const FString ReducersSourceName = FString::Printf(TEXT("%sReducers.cpp"), *DatabaseNamePascal);
    const FString ReducersSourcePath = SourceOutputDir / ReducersSourceName;
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

bool RawModuleDefFromCli(const FString &DatabaseName, FString &Output)
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

	Output = StdOut;
	// Return SUCCESS
	return true;
}

bool RawModuleDefFromHttp(
	const FString& ServerURL,
	const FString& DatabaseName,
	FString& OutRawModuleDef)
{
	return RawModuleDefFromCli(DatabaseName, OutRawModuleDef);
	/*
	SpacetimeDB::String DBName = TCHAR_TO_UTF8(*DatabaseName);
	SpacetimeDB::Database::Client Client =
		SpacetimeDB::Database::Client(DBName, "http://localhost:3000");

	const auto Response = Client.GetSchema();
	if (!SpacetimeDB::IsValid(Response))
	{
		auto StdbError = SpacetimeDB::GetErrorMessage(Response);
		FString UEError(UTF8_TO_TCHAR(StdbError.c_str())); 
		UE_LOG(LogTemp, Error, TEXT("Invalid call to Spacetime GET schema: %s"), *UEError);

		return false;
	}

	const auto Schema = SpacetimeDB::GetResult(Response);

	OutRawModuleDef = UTF8_TO_TCHAR(Schema.Body.c_str());
	
	return true;
	*/

	UE_LOG(LogTemp, Error, TEXT("RawModuleDefFromHttp not implemented"))
	
	return false;
}

bool USpacetimeDBEditorHelpers::IsCliAvailable()
{
	int32 ReturnCode;
	FString OutStdOut, OutStdErr;
	
	return FPlatformProcess::ExecProcess(
		TEXT("spacetime"),
		TEXT(""), &ReturnCode, &OutStdOut, &OutStdErr);
}

bool USpacetimeDBEditorHelpers::IsLoggedIn()
{
	int32 ReturnCode;
	FString OutStdOut, OutStdErr;
	
	if (!FPlatformProcess::ExecProcess(
		TEXT("spacetime"),
		TEXT("login show"), &ReturnCode, &OutStdOut, &OutStdErr))
	{
		return false;
	}

	if (ReturnCode != 0) return false;

	return OutStdOut.Contains("You are logged in as ");
}

bool USpacetimeDBEditorHelpers::TryParseSpacetimeLogin(const FString& CliOutput, FSpacetimeCredentials& OutCredentials)
{
	OutCredentials = FSpacetimeCredentials();

	// Break into lines
	TArray<FString> Lines;
	CliOutput.ParseIntoArrayLines(Lines, /*InCullEmpty*/ true);

	const FString IdentityPrefix = TEXT("You are logged in as ");
	const FString TokenPrefix    = TEXT("Your auth token (don't share this!) is ");

	for (const FString& RawLine : Lines)
	{
		FString Line = RawLine.TrimStartAndEnd();

		if (Line.StartsWith(IdentityPrefix))
		{
			// Everything after the prefix is the identity
			OutCredentials.Identity = Line.Mid(IdentityPrefix.Len());
		}
		else if (Line.StartsWith(TokenPrefix))
		{
			// Everything after the prefix is the JWT
			OutCredentials.Token = Line.Mid(TokenPrefix.Len());
		}
	}

	return OutCredentials.IsValid();
}
