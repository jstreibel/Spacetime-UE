// Fill out your copyright notice in the Description page of Project Settings.

#include "StdbPrototype/BlackholioSerialization.h"

#include "BlackholioExportedTypes.stdbgen.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

void ProductStart(const FJsonWriterRef& Writer, TOptional<FString> Key)
{
	if (Key.IsSet())
	{
		const auto TagString = Key.GetValue();
		Writer->WriteArrayStart(*TagString);
	}
	else
	{
		Writer->WriteArrayStart();
	}
}

void ProductEnd(const FJsonWriterRef& Writer, TOptional<FString> Key)
{
	Writer->WriteArrayEnd();
}

void SumStart(const FJsonWriterRef& Writer, TOptional<FString> Key)
{
	if (Key.IsSet())
	{
		const auto TagString = Key.GetValue();
		Writer->WriteObjectStart(*TagString);
	}

	else
	{
		Writer->WriteObjectStart();
	}
}

void SumEnd(const FJsonWriterRef& Writer, const TOptional<FString>& Key)
{
	Writer->WriteObjectEnd();
}

void SerializeLastSplitTime(
	const FLastSplitTime& SpacetimeProduct,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key)
{	
	// Product:
	ProductStart(Writer, Key);
	
	{
		// I64
		SerializeNumber(SpacetimeProduct.TimestampMicrosSinceUnixEpoch, Writer);
	}
	
	ProductEnd(Writer, Key);
}

void SerializeInterval(
	const FInterval& SpacetimeProduct,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key)
{
	// Product:
	ProductStart(Writer, Key);

	{
		// I64
		SerializeNumber(SpacetimeProduct.TimeDurationMicros, Writer);
	}

	ProductEnd(Writer, Key);
}

void SerializeTime(
	const FTime& SpacetimeProduct,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key)
{	
	// Product:
	ProductStart(Writer, Key);
	
	{
		// I64
		SerializeNumber(SpacetimeProduct.TimestampMicrosSinceUnixEpoch, Writer);
	}
	
	ProductEnd(Writer, Key);
}

void SerializeScheduledAt(
	const FScheduledAt& SpacetimeSum,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key)
{
	// Sum:
	SumStart(Writer, Key);
	
	switch (SpacetimeSum.Tag)
	{
	case EScheduledAt_Tags::Interval: // Interval
		SerializeInterval(SpacetimeSum.Interval, Writer,FString("interval"));
		break;
	case EScheduledAt_Tags::Time: // Time
		SerializeTime	 (SpacetimeSum.Time,	 Writer,FString("time"));
		break;
	case EScheduledAt_Tags::None:
		UE_LOG(LogTemp, Error, TEXT("Unhandled internal error: invalid scheduled_at tag"));
	}

	SumEnd(Writer, Key);
}

void SerializeCircleDecayTimer(
	const FCircleDecayTimer& SpacetimeProduct,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key)
{
	// Product:
	ProductStart(Writer, Key);
	
	{
		// U64
		SerializeNumber(SpacetimeProduct.ScheduledId, Writer);

		// Sum
		SerializeScheduledAt(SpacetimeProduct.ScheduledAt, Writer);
	}
	
	ProductEnd(Writer, Key);
}

void DeserializeCircleDecayTimer(FCircleDecayTimer& Timer, FJsonWriterRef Writer)
{
	UE_LOG(LogTemp, Error, TEXT("Not implemented"));
}
