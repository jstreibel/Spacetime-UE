#include "SpacetimeDBEditorModule.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Misc/MessageDialog.h"
#include "SpacetimeStatusTab.h"
#include "CLI/SpacetimeCLIHelper.h"

static const FName UtilsTabName("SpacetimeDBUtils");

#define LOCTEXT_NAMESPACE "FSpacetimeDBEditorModule"

void FSpacetimeDBEditorModule::StartupModule()
{
    // 1. Register the tab
    const auto
    OnSpawnTab = FOnSpawnTab::CreateRaw(this, &FSpacetimeDBEditorModule::SpawnGeneralTab); 
    auto
    TabSpawnerEntry = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UtilsTabName, OnSpawnTab);
    TabSpawnerEntry
    .SetDisplayName(LOCTEXT("GeneratorTabTitle", "SpacetimeDB"))
    .SetMenuType(ETabSpawnerMenuType::Hidden);

    // 2. Add menu entry under Window > Developer Tools
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
        {
            UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
            // FToolMenuSection& Section = Menu->FindOrAddSection("Developer");

            FToolMenuSection& SpacetimeSection = Menu->AddSection(
                "SpacetimeDB",                               // Section name (must be unique)
                LOCTEXT("SpacetimeDBSection", "SpacetimeDB") // What the user sees
            );
            
            SpacetimeSection.AddMenuEntry(
                "OpenSpacetimeDBUtils",
                LOCTEXT("OpenUtils", "Spacetime Utils"),
                LOCTEXT("OpenUtilsTooltip", "Open the SpacetimeDB utilities tab."),
                FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("ClassIcon.DataTable")),
                FUIAction(FExecuteAction::CreateLambda([]
                {
                    FGlobalTabmanager::Get()->TryInvokeTab(UtilsTabName);
                }))
            );
        })
    );
}

void FSpacetimeDBEditorModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UtilsTabName);
}

TSharedRef<SDockTab> FSpacetimeDBEditorModule::SpawnGeneralTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
    .TabRole(ETabRole::NomadTab)
    [
        SNew(SSpacetimeStatusTab)
        .RefreshInterval(3.0f)  // every 3 seconds
    ];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpacetimeDBEditorModule, SpacetimeDBEditor)
