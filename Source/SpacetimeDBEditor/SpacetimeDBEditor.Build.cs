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
        string SDKRoot          = Path.Combine(PluginRoot, "../../ExternalDependencies/SpacetimeSDK");
        string SDKIncludeDir    = Path.Combine(SDKRoot, "include");

        PublicIncludePaths.Add(SDKIncludeDir);

        // 2) Add the .a file (built by hand) so UBT will link it
        //    Adjust this path if your .a lives somewhere else (e.g. a subfolder per platform).
        string SDKLibPath       = Path.Combine(SDKRoot, "build", "libSpacetimeDBSdk.a");
        PublicAdditionalLibraries.Add(SDKLibPath);

        // 3) If you ever need any compile‐time defines or flags for the SDK, you can add them here:
        // PublicDefinitions.Add("STDB_SOME_DEFINE=1");

        // 4) (Optional) If the SDK .a depends on other static libraries (e.g. cpr, etc.), 
        //    you’d also add them via PublicAdditionalLibraries.Add(...)
    }
}
