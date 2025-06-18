using System.Collections.Generic;
using UnrealBuildTool;

public class SpacetimeDBEditor : ModuleRules
{
    public SpacetimeDBEditor(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "Json",
            "JsonUtilities",
            "SpacetimeDBRuntime",
            "Blutility"
        });
        
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "InputCore",
            "EditorStyle",
            "LevelEditor",
            "ToolMenus",
            "PropertyEditor",
            "Projects",
            
            "SpacetimeDBCore"
        });
    }
}
