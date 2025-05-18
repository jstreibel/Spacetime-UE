// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class SpacetimeDB : ModuleRules
{
	public SpacetimeDB(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		var projectGenPublicPath = Path.GetFullPath(Path.Combine(
			ModuleDirectory, "Public", "Generated"
		));
		var projectGenPrivatePath = Path.GetFullPath(Path.Combine(
			ModuleDirectory, "Private", "Generated"
		));
		PublicIncludePaths.Add(projectGenPublicPath);
		PrivateIncludePaths.Add(projectGenPrivatePath);
		
		PublicIncludePaths.AddRange(
			new string[] {
				// TODO automatically add this?
				Path.Combine(ModuleDirectory, "Public/Generated") 
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				// "Slate",
				// "SlateCore",
				
				"Json",			// Add Unreal JSON parser for std output from Spacetime CLI...
				"JsonUtilities" // ... and a couple extra helpers
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
