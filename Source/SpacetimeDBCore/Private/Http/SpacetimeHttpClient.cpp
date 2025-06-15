// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacetimeHttpClient.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HttpModule.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"

USpacetimeHttpClient* USpacetimeHttpClient::CreateIdentity(const FString& ServerURI)
{
	USpacetimeHttpClient* Node = NewObject<USpacetimeHttpClient>();
	Node->URL = ServerURI / TEXT("v1/identity");
	return Node;
}

void USpacetimeHttpClient::Activate()
{
	Super::Activate();

	// Build the HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetVerb(TEXT("POST"));
	Req->SetURL(URL);
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Bind our handler
	Req->OnProcessRequestComplete().BindUObject(this, &USpacetimeHttpClient::HandleResponse);

	// Fire and forget: UE will call HandleResponse when done
	Req->ProcessRequest();
}

void USpacetimeHttpClient::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// Ensure any Blueprint events fire on the Game Thread
	AsyncTask(ENamedThreads::GameThread, [this, Response, bWasSuccessful]()
	{
		if (!bWasSuccessful || !Response.IsValid())
		{
			OnError.Broadcast(TEXT("Network error or invalid response"));
		}
		else if (Response->GetResponseCode() != EHttpResponseCodes::Ok)
		{
			OnError.Broadcast(FString::Printf(TEXT("HTTP %d"), Response->GetResponseCode()));
		}
		else
		{
			// Parse JSON
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (TSharedPtr<FJsonObject> Root; FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
			{
				if (Root->HasField(TEXT("identity")) && Root->HasField(TEXT("token")))
				{
					FIdentityInfo Info;
					Info.Id    = Root->GetStringField(TEXT("identity"));
					Info.Token = Root->GetStringField(TEXT("token"));
					OnSuccess.Broadcast(Info);
				}
				else
				{
					OnError.Broadcast(TEXT("Missing ‘identity’ or ‘token’ in JSON"));
				}
			}
			else
			{
				OnError.Broadcast(TEXT("Failed to parse JSON"));
			}
		}

		// Let UE gc this object when done
		SetReadyToDestroy();
	});
}
