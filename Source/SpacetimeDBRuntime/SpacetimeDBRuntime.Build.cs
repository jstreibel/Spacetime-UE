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
			// TODO automatically add this?
			Path.Combine(ModuleDirectory, "Public/StdbGenerated") 
			// ... add public include paths required here ...
		});
		
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			// ... add other public dependencies that we statically link with here ...
		});
			
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"CoreUObject",
			"Engine",
			// "Slate",
			// "SlateCore",
			"Json",			// Add Unreal JSON parser for std output from Spacetime CLI...
			"JsonUtilities" // ... and a couple extra helpers
		});
	}
}
