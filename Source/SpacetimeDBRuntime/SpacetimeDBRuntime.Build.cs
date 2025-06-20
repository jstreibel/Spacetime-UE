// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class SpacetimeDBRuntime : ModuleRules
{
	public SpacetimeDBRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Private/StdbGenerated")
		});
		
		PublicIncludePaths.AddRange(new string[] {
			// Path.GetFullPath(Path.Combine(ModuleDirectory, "Public", "StdbGenerated")),
			Path.Combine(ModuleDirectory, "Public/StdbGenerated") 
		});
		
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "SpacetimeDBCore",
		});
			
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"CoreUObject",
			"Engine",
			// "Slate",
			// "SlateCore",
			"Json",			// Add Unreal JSON parser for std output from Spacetime CLI...
			"JsonUtilities", // ... and a couple extra helpers
			"SpacetimeDBCore"
		});
	}
}
