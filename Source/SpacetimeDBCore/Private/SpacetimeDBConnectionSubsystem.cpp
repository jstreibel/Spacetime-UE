#include "SpacetimeDBConnectionSubsystem.h"
#include "AuthSubsystem/SpacetimeAuthSubsystem.h"

#include "HttpModule.h"

#include "Interfaces/IHttpResponse.h"
#include "Misc/App.h"


void USpacetimeAsyncReducerBase::Setup(
	UObject* WorldContext,
	const FString& ReducerName,
	const FString& PayloadJson)
{
	this->ContextObject = WorldContext;
	this->ReducerName   = ReducerName;
	this->PayloadJson   = PayloadJson;
}

void USpacetimeAsyncReducerBase::Activate()
{
	Super::Activate();
	
	// Grab your subsystem
	const UWorld* World = GEngine->GetWorldFromContextObjectChecked(ContextObject);
	const auto* Conn = World
	  ->GetGameInstance()
	  ->GetSubsystem<USpacetimeDBConnectionSubsystem>();

	FOnHttpSuccess OnSuccess_Callback = [this](FHttpResponsePtr Resp){
		OnSuccess.Broadcast();
		SetReadyToDestroy();        // cleans up the async action

		if (Resp->GetContentAsString().Len() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[SpacetimeDB] Reducer '%s' call success! Response: %s"), *ReducerName, *Resp->GetContentAsString());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[SpacetimeDB] Reducer '%s' call success!"), *ReducerName);
		}
	};

	FOnHttpError OnError_Callback = [this](FHttpResponsePtr Resp, bool bWasSuccessful){
		const FString Msg = Resp.IsValid() ? Resp->GetContentAsString() : TEXT("Network error");
		OnFailure.Broadcast(Msg);
		SetReadyToDestroy();

		if (bWasSuccessful)
		{
			UE_LOG(LogTemp, Warning, TEXT("[SpacetimeDB] Reducer '%s' call error: %s"), *ReducerName, *Resp->GetContentAsString())
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[SpacetimeDB] HTTP request for Reducer '%s' call error: %s"), *ReducerName, *Resp->GetContentAsString());
		}
	};	

	const FString Endpoint = "/v1/database/" + Conn->SpacetimeDBModuleName + "/call/" + ReducerName;
	constexpr auto PostVerb = USpacetimeDBConnectionSubsystem::Post;
	
	Conn->AsyncHttpRequest(Endpoint, PostVerb, PayloadJson, OnSuccess_Callback, OnError_Callback);
	
}

void USpacetimeDBConnectionSubsystem::InitializeContext(const FString& ServerURI, const FString& ModuleName,
                                                        const FIdentityInfo& IdentityInfo)
{
	SpacetimeDBServerURI    = ServerURI;
	SpacetimeDBModuleName   = ModuleName;
	SpacetimeDBIdentityInfo = IdentityInfo;

	// TODO: validate connection parameters
	UE_LOG(LogTemp, Log, TEXT("[SpacetimeDB] Initialized connection to %s with module '%s' and user id '%s'"),
		*SpacetimeDBServerURI, *SpacetimeDBModuleName, *SpacetimeDBIdentityInfo.Id);
}

void USpacetimeDBConnectionSubsystem::InvokeReducer(const FString& ReducerName, const FString& JsonPayload) const
{
	const FString Endpoint = "/v1/database/" + SpacetimeDBModuleName + "/call/" + ReducerName;
	const FString ContentJson = JsonPayload;

	FOnHttpSuccess OnSuccess = [](const FHttpResponsePtr& Response)
	{
		UE_LOG(LogTemp, Log, TEXT("[SpacetimeDB] %s"), *Response->GetContentAsString());
	};
	FOnHttpError OnError = [](const FHttpResponsePtr& Response, bool bWasSuccessful)
	{
		if (!bWasSuccessful)
		{
			UE_LOG(LogTemp, Error, TEXT("[SpacetimeDB] HTTP request failed. Response: %s"), *Response->GetContentAsString().Replace(TEXT("\n"), TEXT("")));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[SpacetimeDB] HTTP request succeeded, but response is invalid. Response: %s"), *Response->GetContentAsString());
		}
	};
	
	// Note: JsonPayload must be an array of arguments (SATS-JSON) to the reducer
	AsyncHttpRequest(Endpoint, Post, ContentJson, OnSuccess, OnError);
}


void USpacetimeDBConnectionSubsystem::AsyncHttpRequest(
	const FString& Endpoint,
	const EHttpVerbs& Verb,
	const FString& ContentJson,
	FOnHttpSuccess& OnSuccess,
	FOnHttpError& OnError) const
{
	FString VerbString;
	switch (Verb)
	{
		case Get:	 VerbString = TEXT("GET");	  break;
		case Post:	 VerbString = TEXT("POST");	  break;
		case Put:	 VerbString = TEXT("PUT");	  break;
		case Delete: VerbString = TEXT("DELETE"); break;
		// PATCH
	}
	
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(SpacetimeDBServerURI / Endpoint);;
	Req->SetVerb(VerbString);
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetHeader(TEXT("Authorization"), "Bearer " + SpacetimeDBIdentityInfo.AuthToken);

	if (!ContentJson.IsEmpty()) {
		Req->SetHeader(TEXT("Content-Length"), FString::FromInt(ContentJson.Len()));
		Req->SetContentAsString(ContentJson);
	}

	Req->OnProcessRequestComplete().BindLambda(
		[OnSuccess = MoveTemp(OnSuccess), OnError = MoveTemp(OnError)]
		(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
			{
				OnSuccess(Response);
			}
			else
			{
				OnError(Response, bWasSuccessful);
			}
		});

	Req->ProcessRequest();
}
