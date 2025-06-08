#include "SpacetimeDBEditorModule.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Misc/MessageDialog.h"
#include "SpacetimeStatusTab.h"

static const FName GeneratorTabName("SpacetimeDBGenerator");

#define LOCTEXT_NAMESPACE "FSpacetimeDBEditorModule"

void FSpacetimeDBEditorModule::StartupModule()
{
    // 1. Register the tab
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GeneratorTabName,
        FOnSpawnTab::CreateRaw(this, &FSpacetimeDBEditorModule::SpawnGeneralTab))
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
                "OpenSpacetimeDBGenerator",
                LOCTEXT("OpenGenerator", "Spacetime Utils"),
                LOCTEXT("OpenGeneratorTooltip", "Open the SpacetimeDB utilities tab."),
                FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("ClassIcon.DataTable")),
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

TSharedRef<SDockTab> FSpacetimeDBEditorModule::SpawnGeneralTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
    .TabRole(ETabRole::NomadTab)
    [
        SNew(SSpacetimeStatusTab)
        .RefreshInterval(3.0f)  // every 3 seconds
    ];

    /*
    return SNew(SDockTab)
    .TabRole(NomadTab)
    [
        SNew(SVerticalBox)
        
        // ─── Row 1: "CLI: available/unavailable" ─────────────────────────────────────
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("CLI: ")))
            ]

            // Value (green if available, red otherwise)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text( FText::FromString( bCliAvailable
                                          ? TEXT("available")
                                          : TEXT("unavailable") ) )
                .ColorAndOpacity( bCliAvailable
                                  ? FSlateColor(FLinearColor::Green)
                                  : FSlateColor(FLinearColor::Red) )
            ]
        ]

        // ─── Row 2: "Status: logged in/offline" ────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4)
        [
            SNew(SHorizontalBox)

            // Label
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Status: ")))
            ]

            // Value (green if logged in, red otherwise)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text( FText::FromString( bIsLoggedIn
                                          ? TEXT("logged in")
                                          : TEXT("offline") ) )
                .ColorAndOpacity( bIsLoggedIn
                                  ? FSlateColor(FLinearColor::Green)
                                  : FSlateColor(FLinearColor::Red) )
            ]
        ]
        
        // 1) Labels + Text Boxes
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("DBUrl", "Server URL:"))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SAssignNew(ServerURLTextBox, SEditableTextBox)
            .Text(LOCTEXT("STDBServerURLText", "http://localhost.com:3000"))
            .HintText(LOCTEXT("DBServerURLHint", "e.g. http://localhost.com:3000"))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("DBNameLabel", "Database Name:"))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SAssignNew(DatabaseNameTextBox, SEditableTextBox)
            .HintText(LOCTEXT("DBNameHint", "e.g. quickstart-chat"))
        ]
        

        // 2) Generate Button
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SButton)
            .Text(LOCTEXT("GenerateButton", "Generate Code Reflection"))
            .ToolTipText(LOCTEXT("GenerateTooltip",
                "Fetches RawModuleDef from Given Spacetime Module and Generates Unreal Code Reflection"))
            .OnClicked_Lambda([this]() -> FReply
            {
                if (!DatabaseNameTextBox.IsValid())
                {
                    FMessageDialog::Open(
                        EAppMsgType::Ok,
                        LOCTEXT("STDBGenerateMissedDatabaseName",
                            "🛑 Missing field: database name"));
                    return FReply::Handled();
                }

                if (!ServerURLTextBox.IsValid())
                {
                    FMessageDialog::Open(
                        EAppMsgType::Ok,
                        LOCTEXT("STDBGenerateMissedServerURL",
                            "🛑 Missing field: server URL"));
                    return FReply::Handled();
                }

                const FString ServerURL = ServerURLTextBox->GetText().ToString();
                
                const FString DBName = DatabaseNameTextBox->GetText().ToString();
                FString OutDir, ErrorMsg;

                const bool bOK = USpacetimeDBEditorHelpers::GenerateCxxUnrealCodeFromSpacetimeDB(
                    ServerURL, DBName, OutDir, ErrorMsg
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
    */
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpacetimeDBEditorModule, SpacetimeDBEditor)
