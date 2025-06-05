using System.IO;
using UnrealBuildTool;

public class SpacetimeDBEditor : ModuleRules
{
    public SpacetimeDBEditor(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        /*
        var projectGenPublicPath = Path.GetFullPath(Path.Combine(
            ModuleDirectory, "Public", "Generated"
        ));
        var projectGenPrivatePath = Path.GetFullPath(Path.Combine(
            ModuleDirectory, "Private", "Generated"
        ));
        PublicIncludePaths.Add(projectGenPublicPath);
        PrivateIncludePaths.Add(projectGenPrivatePath);
        */
        
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "UnrealEd",
            
            "Json", "JsonUtilities",
            
            "SpacetimeDB", "Blutility"
        });
        
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate", "SlateCore", "EditorStyle", "LevelEditor", "ToolMenus",
            "PropertyEditor",
            "Projects"
        });
        
        // 1) Point the build to the SDK’s “include/” folder
        //    We assume the Build.cs lives in:
        //      <MyUnrealProject>/Plugins/MyPlugin/Source/MyPlugin/
        //    so “ModuleDirectory” is:
        //      <MyUnrealProject>/Plugins/MyPlugin/Source/MyPlugin
        string PluginRoot       = ModuleDirectory;
        string SDKRoot          = Path.Combine(PluginRoot, "../ExternalDependencies/SpacetimeSDK");
        string SDKIncludeDir    = Path.Combine(SDKRoot, "include");

        PublicIncludePaths.Add(SDKIncludeDir);

        // 2) Add the .a file (built by hand) so UBT will link it
        //    Adjust this path if your .a lives somewhere else (e.g. a subfolder per platform).
        if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicAdditionalLibraries.Add(Path.Combine(SDKRoot, "build", "libSpacetimeDBSdk.a"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicAdditionalLibraries.Add(Path.Combine(SDKRoot, "build", "libSpacetimeDBSdk.a"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // if you ever produce a .lib on Windows, e.g. spacetimedb.lib
            PublicAdditionalLibraries.Add(Path.Combine(SDKRoot, "build", "libSpacetimeDBSdk.lib"));
        }

        // 3) (Optional) If we ever need any compile‐time defines or flags for the SDK, we can add them here:
        // PublicDefinitions.Add("STDB_SOME_DEFINE=1");

        // 4) (Optional) If the SDK .a depends on other static libraries (e.g. cpr, etc.), 
        //    we’d also add them via PublicAdditionalLibraries.Add(...)
    }
}
