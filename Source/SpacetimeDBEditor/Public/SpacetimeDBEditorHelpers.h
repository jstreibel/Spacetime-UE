// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpacetimeDBEditorHelpers.generated.h"

/**
 * 
 */
UCLASS()
class SPACETIMEDBEDITOR_API USpacetimeDBEditorHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** 
	 * Fetches `spacetime describe --json`, parses tables & fields 
	 * and writes USTRUCT headers under Public/Generated/.
	 */
	UFUNCTION(CallInEditor, Category="SpacetimeDB|Generators")
	static bool GenerateCxxUnrealCodeFromSpacetimeDB(
		const FString& ServerURL,
		const FString& DatabaseName,
		FString& OutFullPath,
		FString& OutError);

};
