#include "ReducersProto.h"

#include "SpacetimeDBConnectionSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "StdbGenerated/BlackholioSerialization.stdbgen.h"


UCallBlackholioCircleDecayReducer* UCallBlackholioCircleDecayReducer::CircleDecayReducer(
	UObject* WorldContextObject,
	const FCircleDecayTimer& Timer)
{	
	FString JsonPayload;
	const auto WriterRef = FWriterFactory::Create(&JsonPayload);
	Blackholio::SerializeCircleDecayTimer(Timer, WriterRef);
	WriterRef->Close();

	// Json Payload must be an array, even for a single argument:
	JsonPayload = FString::Printf(TEXT("[%s]"), *JsonPayload);

	UE_LOG(LogTemp, Log, TEXT("[SpacetimeDB] Serialized 'CircleDecayTimer': %s"), *JsonPayload.Replace(TEXT("\n"), TEXT("")));
    
	// TODO: call SpacetimeDB client reducer 'CircleDecay'
	const FString ReducerName = TEXT("CircleDecay");

	auto* Node = NewObject<UCallBlackholioCircleDecayReducer>();
	Node->Setup(WorldContextObject, ReducerName, JsonPayload);
	
	return Node;

}