// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#define BLACKHOLIO_SERIALIZATION_ENABLED

#ifdef BLACKHOLIO_SERIALIZATION_ENABLED

#include "CoreMinimal.h"
#include "BlackholioExportedTypes.stdbgen.h"
#include "SpacetimeRuntimeSDK.h"
#include "Schema/RawModuleDefSchema.h"

void SerializeLastSplitTime(
	const FLastSplitTime& SpacetimeProduct,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key = {});

void SerializeInterval(
	const FInterval& SpacetimeProduct,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key = {});

void SerializeTime(
	const FTime& SpacetimeProduct,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key = {});

void SerializeScheduledAt(
	const FScheduledAt& SpacetimeSum,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key = {});

void SerializeCircleDecayTimer(
	const FCircleDecayTimer& Timer,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key = {});

void SerializeConsumeEntityTimer(
	const FConsumeEntityTimer& Timer,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key = {});

void SerializeMoveAllPlayersTimer(
	const FMoveAllPlayersTimer& Timer,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key = {});

void SerializeSpawnFoodTimer(
	const FSpawnFoodTimer& Timer,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key = {});

void SerializeDbVector2(
	const FDbVector2& Vector,
	const FJsonWriterRef& Writer,
	const SATS::FOptionalString& Key = {});

void DeserializeCircleDecayTimer(
	FCircleDecayTimer& Timer,
	FJsonWriterRef Writer);

#endif
