// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacetimeBlueprintLibrary.h"

#include "HAL/PlatformProcess.h" // for launching external (stdb cli, in this case) processes
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonValue.h"

static bool RunSpacetimeDB(const FString& Args, FString& OutFullOutput, int32& OutReturnCode)
{
	FString StdOut, StdErr;
	if (!FPlatformProcess::ExecProcess(
		TEXT("spacetime"), *Args, &OutReturnCode, &StdOut, &StdErr))
	{
		OutFullOutput = StdErr.IsEmpty() ? TEXT("Failed to launch CLI.") : StdErr;
		OutFullOutput += TEXT("\n") + StdOut;
		return false;
	}

	if (OutReturnCode != 0)
	{
		if (!StdErr.IsEmpty())
		{
			OutFullOutput = StdErr + TEXT("\n") + StdOut;
			return false;			
		}
	}

	if (!StdErr.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Spacetime] CLI output:\n%s"), *StdOut);
	}
	
	OutFullOutput = StdOut;
	return true;
}

bool USpacetimeBlueprintLibrary::DescribeDatabase(
	const FString& DatabaseName,
	TArray<FString>& Tables,
	TArray<FString>& Reducers,
	FString& OutError)
{
	if (DatabaseName.IsEmpty())
	{
		OutError = TEXT("Database name is empty");
		UE_LOG(LogTemp, Error, TEXT("[Spacetime] %s"), *OutError);
		
		return false;
	}
	
	// TODO: check if 'spacetime' binary available
	// TODO: check if server running, logged, etc.	
	
	int32 ReturnCode;
	FString FullOutput;
	if (!RunSpacetimeDB(TEXT("describe --json ") + DatabaseName, FullOutput, ReturnCode))
	{
		OutError = FullOutput;
		UE_LOG(LogTemp, Error, TEXT("[Spacetime] %s"), *OutError);
		return false;
	}

	// 3. Parse JSON object
	// Filter out any warning lines
	auto JsonText = FullOutput;
	TSharedPtr<FJsonObject> JsonObj;
	if (auto Reader = TJsonReaderFactory<>::Create(JsonText);
		!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		// Grab parser error info;
		const FString ErrorMessage = Reader->GetErrorMessage();
		OutError = FString::Printf(
			TEXT("JSON parse failed: %s"), *ErrorMessage);
		UE_LOG(LogTemp, Error, TEXT("[Spacetime] %s"), *OutError);
		UE_LOG(LogTemp, Error, TEXT("[Spacetime] While parsing JSON output from CLI:\n%s"), *JsonText);
		return false;
	}

	for (const auto& Entry : JsonObj->Values)
	{
		UE_LOG(LogTemp, Log, TEXT("[Spacetime] DB entry: %s"), *Entry.Key);
	}
	
	if (!JsonObj->HasField(TEXT("tables")))
	{
		UE_LOG(LogTemp, Error, TEXT("[Spacetime] JSON output from CLI does not contain 'tables' field"));
		return false;
	}
	if (!JsonObj->HasField(TEXT("reducers")))
	{
		UE_LOG(LogTemp, Error, TEXT("[Spacetime] JSON output from CLI does not contain 'reducers' field"));
		return false;
	}

	for (auto TablesArray = JsonObj->GetArrayField(TEXT("tables"));
		 const auto& Table : TablesArray)
	{
		Tables.Add(Table->AsObject()->GetStringField(TEXT("name")));
	}

	for (auto& Reducer : JsonObj->GetArrayField(TEXT("reducers")))
	{
		Reducers.Add(Reducer->AsObject()->GetStringField(TEXT("name")));
	}

	//UE_LOG(LogTemp, Warning, TEXT("SpacetimeCLI initialized"));
	//UE_LOG(LogTemp, Warning, TEXT("Something	` might be wrong: %s"), *MyStringVar);
	//UE_LOG(LogTemp, Error, TEXT("Failed to run describe --json"));

	// for (auto &ReducersList = JsonObj->GetArrayField(TEXT("reducers"));
	// 	auto &Reducer : ReducersList)
	// {
	// 	Reducers.Add(Reducer->AsString());
	// }
	

	return true;
}

