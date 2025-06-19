// Fill out your copyright notice in the Description page of Project Settings.

#include "StdbPrototype/BlackholioSerialization.h"

#ifdef BLACKHOLIO_SERIALIZATION_ENABLED

#include "BlackholioExportedTypes.stdbgen.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

void SerializeLastSplitTime(
	const FLastSplitTime& SpacetimeProduct,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key)
{	
	// Product:
	ProductStart(Writer, Key);
	
	{
		// I64
		SerializeNumberOrString(SpacetimeProduct.TimestampMicrosSinceUnixEpoch, Writer);
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
		SerializeNumberOrString(SpacetimeProduct.TimeDurationMicros, Writer);
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
		SerializeNumberOrString(SpacetimeProduct.TimestampMicrosSinceUnixEpoch, Writer);
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
		SerializeInterval(SpacetimeSum.Interval, Writer,FString("Interval"));
		break;
	case EScheduledAt_Tags::Time: // Time
		SerializeTime	 (SpacetimeSum.Time,	 Writer,FString("Time"));
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
		SerializeNumberOrString(SpacetimeProduct.ScheduledId, Writer);

		// Sum
		SerializeScheduledAt(SpacetimeProduct.ScheduledAt, Writer);
	}
	
	ProductEnd(Writer, Key);
}

void SerializeConsumeEntityTimer(const FConsumeEntityTimer& Timer, const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key)
{
	UE_LOG(LogTemp, Error, TEXT("Not implemented"));
}

void SerializeMoveAllPlayersTimer(const FMoveAllPlayersTimer& Timer, const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key)
{
	UE_LOG(LogTemp, Error, TEXT("Not implemented"));
}

void SerializeSpawnFoodTimer(const FSpawnFoodTimer& Timer, const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key)
{
	UE_LOG(LogTemp, Error, TEXT("Not implemented"));
}

void SerializeDbVector2(const FDbVector2& Vector, const FJsonWriterRef& Writer, const SATS::FOptionalString& Key)
{
	UE_LOG(LogTemp, Error, TEXT("Not implemented"));
}

void DeserializeCircleDecayTimer(FCircleDecayTimer& Timer, FJsonWriterRef Writer)
{
	UE_LOG(LogTemp, Error, TEXT("Not implemented"));
}

#endif
