#pragma once

#include "CoreMinimal.h"

/**
 * Internal helper for writing generated code files to disk.
 */
class FCodeFileWriter
{
public:
	/**
	 * Writes the given text to FilePath, creating directories if needed.
	 * @param FilePath  Full path to the target file (including filename and extension).
	 * @param OutCode   The text content to write.
	 * @param OutError  Receives an error message on failure.
	 * @return true if the file was written successfully.
	 */
	static bool WriteFile(
		const FString& FilePath,
		const FString& OutCode,
		FString& OutError
	);
};
