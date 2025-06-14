#include "SpacetimeDBInterface.h"

#include "AuthSubsystem/SpacetimeAuthSubsystem.h"

#include "HAL/IConsoleManager.h"  // in some engine versions
#include "Misc/OutputDevice.h"	  // for GLog
#include "Misc/App.h"

FString USpacetimeDBInterface::GetUserToken(const FString& ServerURI)
{
	return FString();
}
