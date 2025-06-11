#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreMinimal.h"
#include "BlackholioTypespace.h"
#include "BlackholioReducers.generated.h"


UCLASS()
class SPACETIMEDBRUNTIME_API UBlackholioReducers : public UBlueprintFunctionLibrary {

   GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void CircleDecay(const FCircleDecayTimer& Timer);

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void CircleRecombine(const FCircleRecombineTimer& Timer);

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void Connect();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void ConsumeEntity(const FConsumeEntityTimer& Request);

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void Disconnect();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void EnterGame(const FString& Name);

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void Init();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void MoveAllPlayers(const FMoveAllPlayersTimer& Timer);

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void PlayerSplit();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void Respawn();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void SpawnFood(const FSpawnFoodTimer& Timer);

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void Suicide();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB|Blackholio")
    static void UpdatePlayerInput(const FDbVector2& Direction);

};
