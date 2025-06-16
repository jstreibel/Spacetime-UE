// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacetimeRuntimeSDK.h"

/*
void USpacetimeSum::SerializeToSatsJson(FString& OutJsonPayload) const
{
	
}

void USpacetimeProduct::SerializeToSatsJson(FString& OutJsonPayload) const
{
	for (TFieldIterator<FProperty> It(this->GetClass()); It; ++It)
	{
		UE_LOG(LogTemp, Log, TEXT("Property: %s"), *It->GetName());
	}
}
*/
FString USpacetimeSerialization::SerializeProductToJson(const FSpacetimeProduct& SpacetimeProduct, FString& JsonPayload)
{
	for (TFieldIterator<FProperty> It(SpacetimeProduct.StaticStruct()); It; ++It)
	{
		UE_LOG(LogTemp, Log, TEXT("Property: %s"), *It->GetName());
	}

	return "[]";
}

FString USpacetimeSerialization::SerializeSumToJson(const FSpacetimeSum& SpacetimeSum, FString& JsonPayload)
{
	for (TFieldIterator<FProperty> It(SpacetimeSum.StaticStruct()); It; ++It)
	{
		UE_LOG(LogTemp, Log, TEXT("Property: %s"), *It->GetName());
	}

	return "[]";
}
