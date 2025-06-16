// Fill out your copyright notice in the Description page of Project Settings.

/*
#include "StdbPrototype/BlackholioSerialization.h"

#include "BlackholioExportedTypes.stdbgen.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

void UBlackholioSerialization::SerializeCircleDecayTimer(const FCircleDecayTimer& Timer, FString& OutJsonPayload)
{
	const auto Writer = TJsonWriterFactory<>::Create(&OutJsonPayload);

	// Product:
	Writer->WriteArrayStart();
	{
		// U64
		Writer->WriteValue(Timer.ScheduledId);
		
		// Sum:
		Writer->WriteObjectStart();
		{
			switch (Timer.ScheduledAt.Tag)
			{
			case EScheduledAt_Tags::Interval: // Interval
				// Product:
				Writer->WriteArrayStart("Interval");
				{
					Writer->WriteValue(Timer.ScheduledAt.Interval.TimeDurationMicros);
				}
				Writer->WriteArrayEnd();
				
				break;
			case EScheduledAt_Tags::Time: // Time
				// Product:
				Writer->WriteArrayStart("Time");
				{
					Writer->WriteValue(Timer.ScheduledAt.Time.TimestampMicrosSinceUnixEpoch);
				}
				Writer->WriteArrayEnd();
			case EScheduledAt_Tags::None:
				UE_LOG(LogTemp, Error, TEXT("Unhandled internal error: invalid scheduled_at tag"));
				break;
			}
		}
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();
	Writer->Close();
}

void UBlackholioSerialization::DeserializeCircleDecayTimer(const FCircleDecayTimer& Timer)
{
}
*/