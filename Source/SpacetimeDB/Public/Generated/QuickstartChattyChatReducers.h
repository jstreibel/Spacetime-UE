#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreMinimal.h"
#include "QuickstartChattyChatReducers.generated.h"


UCLASS()
class SPACETIMEDB_API USpacetimeDBReducers : public UBlueprintFunctionLibrary {

   GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
    static void ClientConnected();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
    static void ClientDisconnected();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
    static void SendMessage(const float& Text);

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
    static void SetName(const float& Name);

};
