// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlackholioExportedTypes.stdbgen.h"
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

void DeserializeCircleDecayTimer(
	FCircleDecayTimer& Timer,
	FJsonWriterRef Writer);