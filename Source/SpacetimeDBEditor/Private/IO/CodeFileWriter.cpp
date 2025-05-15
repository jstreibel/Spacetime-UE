#include "IO/CodeFileWriter.h"
#include "Misc/FileHelper.h"
// #include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Logging/LogMacros.h"

bool FCodeFileWriter::WriteFile(
	const FString& FilePath,
	const FString& OutCode,
	FString& OutError
)
{
	// Ensure the directory exists
	const FString Directory = FPaths::GetPath(FilePath);
	if (!IFileManager::Get().DirectoryExists(*Directory))
	{
		if (!IFileManager::Get().MakeDirectory(*Directory, /*Tree=*/ true))
		{
			OutError = FString::Printf(
				TEXT("Failed to create directory: %s"),
				*Directory
			);
			UE_LOG(LogTemp, Error, TEXT("%s"), *OutError);
			return false;
		}
	}

	// Write the file
	if (!FFileHelper::SaveStringToFile(OutCode, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_None))
	{
		OutError = FString::Printf(
			TEXT("Failed to write file: %s"),
			*FilePath
		);
		UE_LOG(LogTemp, Error, TEXT("%s"), *OutError);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully wrote file: %s"), *FilePath);
	return true;
}
