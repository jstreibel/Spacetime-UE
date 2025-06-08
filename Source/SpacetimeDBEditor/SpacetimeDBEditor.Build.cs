using System.Collections.Generic;
using UnrealBuildTool;

public class SpacetimeDBEditor : ModuleRules
{
    public SpacetimeDBEditor(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        var list = new List<string>
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "Json",
            "JsonUtilities",
            "SpacetimeDBRuntime",
            "Blutility"
        };
        PublicDependencyModuleNames.AddRange(list.AsReadOnly());

        var list1 = new List<string>
        {
            "Slate",
            "SlateCore",
            "EditorStyle",
            "LevelEditor",
            "ToolMenus",
            "PropertyEditor",
            "Projects"
        };
        PrivateDependencyModuleNames.AddRange(list1.AsReadOnly());
    }
}
