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
    }
}
