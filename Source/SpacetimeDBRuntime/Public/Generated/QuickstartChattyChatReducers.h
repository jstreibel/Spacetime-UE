#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreMinimal.h"
#include "QuickstartChattyChatReducers.generated.h"


UCLASS()
class SPACETIMEDBRUNTIME_API USpacetimeDBReducers : public UBlueprintFunctionLibrary {

   GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
    static void ClientConnected();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
    static void ClientDisconnected();

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
    static void SendMessage(const FString& Text);

    UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
    static void SetName(const FString& Name);

};
