// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class SpacetimeDBRuntime : ModuleRules
{
	public SpacetimeDBRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		var projectGenPublicPath = Path.GetFullPath(Path.Combine(
			ModuleDirectory, "Public", "StdbGenerated"
		));
		var projectGenPrivatePath = Path.GetFullPath(Path.Combine(
			ModuleDirectory, "Private", "StdbGenerated"
		));
		PublicIncludePaths.Add(projectGenPublicPath);
		PrivateIncludePaths.Add(projectGenPrivatePath);
		
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Public/StdbGenerated") 
		});
		
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
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
