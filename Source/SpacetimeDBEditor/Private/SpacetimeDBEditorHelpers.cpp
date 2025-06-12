// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacetimeDBEditorHelpers.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "CodeGen/SpacetimeDBCodegen.h"
#include "HAL/PlatformProcess.h"
#include "IO/CodeFileWriter.h"
#include "Parser/ModuleDefParser.h"
#include "Schema/SchemaModels.h"

#include <SpacetimeDBEditorHelpers.h>

#include "Config.h"
#include "CodeGen/TypespaceStructIRBuilder.h"

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
	return RawModuleDefFromCli(DatabaseName, OutRawModuleDef);
	// return RawModuleDefFromHttp(ServerURL, DatabaseName, OutRawModuleDef);
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
	const FString GeneratedDirectory = "StdbGenerated";

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

	// 2. Ensure output directory exists
	const FString OutputDir = FPaths::ProjectDir() / TEXT("Plugins/SpacetimeDB/Source/SpacetimeDBRuntime/<Public&Private>") / GeneratedDirectory;
	const FString HeaderOutputDir = FPaths::ProjectDir() / TEXT("Plugins/SpacetimeDB/Source/SpacetimeDBRuntime/Public") / GeneratedDirectory;
	const FString SourceOutputDir = FPaths::ProjectDir() / TEXT("Plugins/SpacetimeDB/Source/SpacetimeDBRuntime/Private") / GeneratedDirectory;
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Creating output directories '%s' and '%s'"), *HeaderOutputDir, *SourceOutputDir);
    if (!IFileManager::Get().MakeDirectory(*HeaderOutputDir, /*Tree=*/ true) | !IFileManager::Get().MakeDirectory(*SourceOutputDir, /*Tree=*/ true))
    {
        OutError = TEXT("Failed to create output directory.");
        UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
        return false;
    }
	

	// 3. Generate typespace structs
	{
		UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating SATS-JSON Typespace Unreal-reflected C++ structs"));
		
		FString ExportedTypesHeaderCode;
		FString InlineTypesHeaderCode;
		// const FString BaseSpacetimeHeaderName = FString::Printf(TEXT("%sTypespace"), *DatabaseNamePascal);
		if (!FSpacetimeDBCodeGen::GenerateTypespaceCode(
			RawModule,						DatabaseName,
			ExportedTypesHeaderCode,		InlineTypesHeaderCode,
			OutError))
		{
			OutError = TEXT("Failed to generate typespace structures: ") + OutError;
			UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
			return false;
		}

		const FString ExportedFilePath =
			HeaderOutputDir / FSpacetimeConfig::MakeExportedTypesCodeFileName(DatabaseName) + ".h";

		const FString InlineFilePath =
			HeaderOutputDir / FSpacetimeConfig::MakeInlineTypesCodeFileName(DatabaseName) + ".h";
		
		if (!FCodeFileWriter::WriteFile(ExportedFilePath, ExportedTypesHeaderCode, OutError))
		{
			OutError = TEXT("Failed to write typespace header file: ") + OutError;
			UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
			return false;
		}

		if (!FCodeFileWriter::WriteFile(InlineFilePath, InlineTypesHeaderCode, OutError))
		{
			OutError = TEXT("Failed to write typespace header file: ") + OutError;
			UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
			return false;
		}

		// FString NicePath = ExportedFilePath;
		// FPaths::NormalizeFilename(NicePath);
		// FPaths::CollapseRelativeDirectories(NicePath);
		// UE_LOG(LogTemp, Log, TEXT("[spacetime] Wrote typespace code to %s"), *NicePath);

		UE_LOG(LogTemp, Log, TEXT("[spacetime] Typespace code output success"));
		
	}
	
	
    // 4. Generate table structs header
	if (0)
	{
		UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating SATS-JSON Tables' Unreal-reflected C++ structs"));
		
		FString TablesHeader;
		const FString BaseTablesHeaderName = FString::Printf(TEXT("%sTables"), *DatabaseNamePascal);
		if (!FSpacetimeDBCodeGen::GenerateTableStructs(RawModule, BaseTablesHeaderName, TablesHeader, OutError))
		{
			OutError = TEXT("Table struct generation failed: ") + OutError;
			UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
			return false;
		}
		const FString TablesHeaderName = BaseTablesHeaderName + ".h";
		const FString TablesHeaderPath = HeaderOutputDir / TablesHeaderName;
		if (!FCodeFileWriter::WriteFile(TablesHeaderPath, TablesHeader, OutError))
		{
			UE_LOG(LogTemp, Error, TEXT("[spacetime] Failed to write tables header '%s': %s"), *TablesHeaderPath, *OutError);
			return false;
		}
		UE_LOG(LogTemp, Log, TEXT("[spacetime] Wrote %s"), *TablesHeaderPath);
	} 

	
    // 5. Generate STDB Reducer functions (header + source)
	{
		UE_LOG(LogTemp, Log, TEXT("[spacetime] Generating reducer Blueprint nodes"));
		
		FString ReducersHeader, ReducersSource;
		if (!FSpacetimeDBCodeGen::GenerateReducerFunctions(
			DatabaseNamePascal,
			RawModule,
			ReducersHeader,
			ReducersSource,
			OutError))
		{
			OutError = TEXT("Reducer function generation failed: ") + OutError;
			UE_LOG(LogTemp, Error, TEXT("[spacetime] %s"), *OutError);
			return false;
		}

		const FString ReducersFilename = FSpacetimeConfig::MakeReducerCodeFileName(DatabaseName); 		
		const FString ReducersHeaderPath = HeaderOutputDir / ReducersFilename + ".h";
		const FString ReducersSourcePath = SourceOutputDir / ReducersFilename + ".cpp";
		
		if (!FCodeFileWriter::WriteFile(ReducersHeaderPath, ReducersHeader, OutError))
		{
			UE_LOG(LogTemp, Error, TEXT("[spacetime] Failed to write reducers header '%s': %s"), *ReducersHeaderPath, *OutError);
			return false;
		}

		FString NicePath = ReducersHeaderPath;
		FPaths::NormalizeFilename(NicePath);
		FPaths::CollapseRelativeDirectories(NicePath);
		UE_LOG(LogTemp, Log, TEXT("[spacetime] Wrote %s"), *NicePath);
		
		if (!FCodeFileWriter::WriteFile(ReducersSourcePath, ReducersSource, OutError))
		{
			UE_LOG(LogTemp, Error, TEXT("[spacetime] Failed to write reducers source '%s': %s"), *ReducersSourcePath, *OutError);
			return false;
		}

		
		NicePath = ReducersSourcePath;
		FPaths::NormalizeFilename(NicePath);
		FPaths::CollapseRelativeDirectories(NicePath);
		UE_LOG(LogTemp, Log, TEXT("[spacetime] Wrote reducer code to %s"), *NicePath);
	}

	
    // 6. Success
    OutFullPath = OutputDir;
	FPaths::ConvertRelativePathToFull(OutFullPath);
    UE_LOG(LogTemp, Log, TEXT("[spacetime] Code generation completed for SpacetimeDB Module '%s'"), *DatabaseName);
	UE_LOG(LogTemp, Log, TEXT("[spacetime] Location: %s"), *OutFullPath);
	
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