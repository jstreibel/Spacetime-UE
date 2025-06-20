#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Http/SpacetimeHttpClient.h"
#include "SpacetimeDBConnectionSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpacetimeOnSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpacetimeOnError, const FString&, ErrorMessage);

using FOnHttpSuccess = TFunction<void(const FHttpResponsePtr& Response)>;
using FOnHttpError   = TFunction<void(const FHttpResponsePtr& Response, bool bWasSuccessful)>;

UCLASS(Abstract)
class SPACETIMEDBCORE_API USpacetimeAsyncReducerBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	void Setup(
	  UObject* WorldContext,
	  const FString& ReducerName,
	  const FString& PayloadJson
	);
	
	virtual void Activate() override;
  
	/** 
   * We mark ContextObject as a UPROPERTY so that the GC knows it could be used async,
   * and won't collect it out inappropriately.
   * TODO: make it a TWeakObjectPtr
   */
	UPROPERTY()
	UObject*            ContextObject;
	FString             ReducerName;
	FString             PayloadJson;

	// these get broadcast in the OnProcessRequestComplete
	UPROPERTY(BlueprintAssignable) FSpacetimeOnSuccess OnSuccess;
	UPROPERTY(BlueprintAssignable) FSpacetimeOnError   OnFailure;
};


UCLASS(Blueprintable)
class SPACETIMEDBCORE_API USpacetimeDBConnectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	enum EHttpVerbs {
		Get,
		Post,
		Put,
		Delete,
		// PATCH
	};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpacetimeDB")
	FString SpacetimeDBServerURI;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpacetimeDB")
	FString SpacetimeDBModuleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpacetimeDB")
	FIdentityInfo SpacetimeDBIdentityInfo;

	UFUNCTION(BlueprintCallable, Category="SpacetimeDB")
	void InitializeContext(const FString& ServerURI, const FString& ModuleName, const FIdentityInfo& IdentityInfo);

	/**
	 * Synchronous call to invoke a reducer.
	 *
	 * @param ReducerName 
	 * @param JsonPayload 
	 */
	UFUNCTION(Category="SpacetimeDB")
	void InvokeReducer(const FString& ReducerName, const FString& JsonPayload) const;

	void AsyncHttpRequest(
		const FString& Endpoint,
		const EHttpVerbs& Verb,
		const FString& ContentJson,
		FOnHttpSuccess& OnSuccess,
		FOnHttpError& OnError) const;
};
