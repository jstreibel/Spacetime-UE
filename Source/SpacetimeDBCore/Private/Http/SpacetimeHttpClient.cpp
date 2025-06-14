// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacetimeHttpClient.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HttpModule.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"

bool USpacetimeHttpClient::CreateIdentity(
	const FString& ServerURI,
	FIdentityInfo& OutIdentity,
	FString& OutError)
{
	// Synchronization event to block until the HTTP response arrives
    FEvent* RequestCompleteEvent = FGenericPlatformProcess::GetSynchEventFromPool(false);

    bool bSucceeded = false;
    FString ErrorMessage;

    // Create the HTTP request
    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetURL( ServerURI / TEXT("v1/identity") );
    HttpRequest->SetHeader("Content-Type", "application/json");

    // Bind response handler
    HttpRequest->OnProcessRequestComplete().BindLambda(
        [&OutIdentity, &bSucceeded, &ErrorMessage, RequestCompleteEvent]
        (FHttpRequestPtr Req, const FHttpResponsePtr& Resp, const bool bWasSuccessful)
        {
            if (!bWasSuccessful || !Resp.IsValid())
            {
                ErrorMessage = TEXT("Failed to POST to /v1/identity: network error or invalid response");
            }
            else if (Resp->GetResponseCode() != EHttpResponseCodes::Ok)
            {
                ErrorMessage = FString::Printf(
                    TEXT("Unhandled status code: %d"),
                    Resp->GetResponseCode()
                );
            }
            else
            {
                // Parse JSON body
                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());

                if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
                {
                    if (JsonObject->HasField(TEXT("identity")))
                    {
                        OutIdentity.Id    = JsonObject->GetStringField(TEXT("id"));
                        OutIdentity.Token = JsonObject->GetStringField(TEXT("token"));
                        bSucceeded = true;
                    }
                    else
                    {
                        ErrorMessage = FString::Printf(
                            TEXT("Response missing 'identity'. Full body: %s"),
                            *Resp->GetContentAsString()
                        );
                    }
                }
                else
                {
                    ErrorMessage = TEXT("Failed to parse JSON response");
                }
            }

            // Wake up the waiting thread
            RequestCompleteEvent->Trigger();
        }
    );

    // Send the request and wait
    HttpRequest->ProcessRequest();
    RequestCompleteEvent->Wait();
    FGenericPlatformProcess::ReturnSynchEventToPool(RequestCompleteEvent);

    // Check result
    if (!bSucceeded)
    {
        OutError = ErrorMessage;
        return false; // empty optional
    }

    return true;
}
