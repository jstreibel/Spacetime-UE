// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacetimeRuntimeSDK.h"

void UAlgebraicSum::Serialize(FString& OutJsonPayload) const
{
	for (TFieldIterator<FProperty> It(this->GetClass()); It; ++It)
	{
		UE_LOG(LogTemp, Log, TEXT("Property: %s"), *It->GetName());
	}
}

void UAlgebraicProduct::Serialize(FString& OutJsonPayload) const
{
	for (TFieldIterator<FProperty> It(this->GetClass()); It; ++It)
	{
		UE_LOG(LogTemp, Log, TEXT("Property: %s"), *It->GetName());
	}
}
