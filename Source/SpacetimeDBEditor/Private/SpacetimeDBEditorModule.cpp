#include "SpacetimeDBEditorModule.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Misc/MessageDialog.h"
#include "SpacetimeDBEditorHelpers.h"  // your CallInEditor helper

static const FName GeneratorTabName("SpacetimeDBGenerator");

#define LOCTEXT_NAMESPACE "FSpacetimeDBEditorModule"

void FSpacetimeDBEditorModule::StartupModule()
{
    // 1. Register the tab
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GeneratorTabName,
        FOnSpawnTab::CreateRaw(this, &FSpacetimeDBEditorModule::SpawnGeneratorTab))
        .SetDisplayName(LOCTEXT("GeneratorTabTitle", "SpacetimeDB Generator"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

    // 2. Add menu entry under Window > Developer Tools
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
        {
            UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
            FToolMenuSection& Section = Menu->FindOrAddSection("Developer");
            Section.AddMenuEntry(
                "OpenSpacetimeDBGenerator",
                LOCTEXT("OpenGenerator", "SpacetimeDB Generator"),
                LOCTEXT("OpenGeneratorTooltip", "Open the SpacetimeDB code generator."),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateLambda([]
                {
                    FGlobalTabmanager::Get()->TryInvokeTab(GeneratorTabName);
                }))
            );
        })
    );
}

void FSpacetimeDBEditorModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GeneratorTabName);
}

TSharedRef<SDockTab> FSpacetimeDBEditorModule::SpawnGeneratorTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
    .TabRole(ETabRole::NomadTab)
    [
        SNew(SVerticalBox)

        // 1) Label + Text Box
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("DBNameLabel", "Database Name:"))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SAssignNew(DatabaseNameTextBox, SEditableTextBox)
            .HintText(LOCTEXT("DBNameHint", "e.g. MyDatabase"))
        ]

        // 2) Generate Button
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SButton)
            .Text(LOCTEXT("GenerateButton", "Generate USTRUCTs"))
            .ToolTipText(LOCTEXT("GenerateTooltip", "Calls GenerateUSTRUCTsFromSchema with the given name"))
            .OnClicked_Lambda([this]() -> FReply
            {
                if (!DatabaseNameTextBox.IsValid())
                {
                    return FReply::Handled();
                }

                const FString DBName = DatabaseNameTextBox->GetText().ToString();
                FString OutDir, ErrorMsg;

                const bool bOK = USpacetimeDBEditorHelpers::GenerateCxxUnrealCodeFromSpacetimeDB(
                    DBName, OutDir, ErrorMsg
                );

                if (!bOK)
                {
                    // Show error
                    FMessageDialog::Open(
                        EAppMsgType::Ok,
                        FText::Format(
                            LOCTEXT("GenerateFailedFmt", "🛑 Generate failed:\n{0}"),
                            FText::FromString(ErrorMsg)
                        )
                    );
                }
                else
                {
                    // Show success and path
                    FMessageDialog::Open(
                        EAppMsgType::Ok,
                        FText::Format(
                            LOCTEXT("GenerateSuccessFmt", "✅ Generate succeeded!\nFiles written to:\n{0}"),
                            FText::FromString(OutDir)
                        )
                    );
                }

                return FReply::Handled();
            })
        ]
    ];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpacetimeDBEditorModule, SpacetimeDBEditor)
