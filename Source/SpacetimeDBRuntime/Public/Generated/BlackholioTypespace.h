#pragma once

#include "CoreMinimal.h"
#include "BlackholioTypespace.generated.h"


/* Provides SATS-JSON U256 support; Unreal UBT lacks uint256 reflection. */
USTRUCT(BlueprintType, Category="SpacetimeDB")
struct  FUInt256 {

    GENERATED_BODY();

    UPROPERTY(BlueprintReadWrite)
    FString Value;

};


/* Provides SATS-JSON I256 support; Unreal UBT lacks int256 reflection. */
USTRUCT(BlueprintType, Category="SpacetimeDB")
struct  FInt256 {

    GENERATED_BODY();

    UPROPERTY(BlueprintReadWrite)
    FString Value;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FCircleDecayTimer {

    GENERATED_BODY();

    /* scheduled_id: U64 */
    UPROPERTY(BlueprintReadWrite)
    int64 ScheduledId;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FCircleRecombineTimer {

    GENERATED_BODY();

    /* scheduled_id: U64 */
    UPROPERTY(BlueprintReadWrite)
    int64 ScheduledId;

    /* player_id: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 PlayerId;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FConsumeEntityTimer {

    GENERATED_BODY();

    /* scheduled_id: U64 */
    UPROPERTY(BlueprintReadWrite)
    int64 ScheduledId;

    /* consumed_entity_id: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 ConsumedEntityId;

    /* consumer_entity_id: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 ConsumerEntityId;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FMoveAllPlayersTimer {

    GENERATED_BODY();

    /* scheduled_id: U64 */
    UPROPERTY(BlueprintReadWrite)
    int64 ScheduledId;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FSpawnFoodTimer {

    GENERATED_BODY();

    /* scheduled_id: U64 */
    UPROPERTY(BlueprintReadWrite)
    int64 ScheduledId;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FDbVector2 {

    GENERATED_BODY();

    /* x: F32 */
    UPROPERTY(BlueprintReadWrite)
    float X;

    /* y: F32 */
    UPROPERTY(BlueprintReadWrite)
    float Y;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FInlineStruct_00 {

    GENERATED_BODY();

    /* __timestamp_micros_since_unix_epoch__: I64 */
    UPROPERTY(BlueprintReadWrite)
    int64 TimestampMicrosSinceUnixEpoch;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FCircle {

    GENERATED_BODY();

    /* entity_id: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 EntityId;

    /* player_id: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 PlayerId;

    /* direction: DbVector2 */
    UPROPERTY(BlueprintReadWrite)
    FDbVector2 Direction;

    /* speed: F32 */
    UPROPERTY(BlueprintReadWrite)
    float Speed;

    UPROPERTY(BlueprintReadWrite)
    FInlineStruct_00 LastSplitTime;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FConfig {

    GENERATED_BODY();

    /* id: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 Id;

    /* world_size: U64 */
    UPROPERTY(BlueprintReadWrite)
    int64 WorldSize;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FEntity {

    GENERATED_BODY();

    /* entity_id: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 EntityId;

    /* position: DbVector2 */
    UPROPERTY(BlueprintReadWrite)
    FDbVector2 Position;

    /* mass: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 Mass;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FFood {

    GENERATED_BODY();

    /* entity_id: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 EntityId;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FInlineStruct_01 {

    GENERATED_BODY();

    /* __identity__: U256 */
    UPROPERTY(BlueprintReadWrite)
    FUInt256 Identity;

};


USTRUCT(BlueprintType, Category="SpacetimeDB|Blackholio")
struct  FPlayer_Blackholio {

    GENERATED_BODY();

    UPROPERTY(BlueprintReadWrite)
    FInlineStruct_01 Identity;

    /* player_id: U32 */
    UPROPERTY(BlueprintReadWrite)
    int32 PlayerId;

    /* name: String */
    UPROPERTY(BlueprintReadWrite)
    FString Name;

};


