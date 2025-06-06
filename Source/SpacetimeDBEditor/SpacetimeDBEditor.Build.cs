using System.IO;
using UnrealBuildTool;

public class SpacetimeDBEditor : ModuleRules
{
    public SpacetimeDBEditor(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
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
