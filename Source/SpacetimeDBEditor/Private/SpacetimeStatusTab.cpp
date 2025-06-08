#include "SpacetimeStatusTab.h"

#include "SpacetimeDBEditorHelpers.h"
#include "Widgets/Text/STextBlock.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Async/Async.h"
#include "CLI/SpacetimeCLIHelper.h"

#define LOCTEXT_NAMESPACE "SpacetimeDBStatusTab"

void SSpacetimeStatusTab::Construct(const FArguments& InArgs)
{
	RefreshInterval = InArgs._RefreshInterval;

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("CLI: ")))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(CLITextBlock, STextBlock)
				.Text(FText::FromString(TEXT("…")))
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("Status: ")))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SAssignNew(StatusTextBlock, STextBlock)
				.Text(FText::FromString(TEXT("…")))
			]
		]

		// ***********************************************
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

	// Kick off the first check
	ScheduleAsyncRefresh();
}

void SSpacetimeStatusTab::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	AccumulatedTime += InDeltaTime;
	if (AccumulatedTime >= RefreshInterval)
	{
		AccumulatedTime = 0.f;
		ScheduleAsyncRefresh();
	}
}

void SSpacetimeStatusTab::ScheduleAsyncRefresh() const
{
	// Capture weak refs so we don’t keep widgets alive if the tab closes
	TWeakPtr<STextBlock> CliWeak    = CLITextBlock;
	TWeakPtr<STextBlock> StatusWeak = StatusTextBlock;

	// Dispatch the blocking work to the thread-pool
	Async(EAsyncExecution::ThreadPool, [CliWeak, StatusWeak]()
	{
		// *** Your blocking calls go here ***
		const bool bCliAvailable = FSpacetimeCLIHelper::IsCliAvailable();
		const bool bLoggedIn     = FSpacetimeCLIHelper::IsLoggedIn();

		// Then marshal back to the game (main) thread
		AsyncTask(ENamedThreads::GameThread, [CliWeak, StatusWeak, bCliAvailable, bLoggedIn]()
		{
			if (auto CLIBlock = CliWeak.Pin())
			{
				CLIBlock->SetText(FText::FromString(bCliAvailable ? TEXT("available")   : TEXT("unavailable")));
				CLIBlock->SetColorAndOpacity(bCliAvailable
					? FSlateColor(FLinearColor::Green)
					: FSlateColor(FLinearColor::Red)
				);
			}
			if (const auto StatusBlock = StatusWeak.Pin())
			{
				StatusBlock->SetText(FText::FromString(bLoggedIn ? TEXT("logged in") : TEXT("offline")));
				StatusBlock->SetColorAndOpacity(bLoggedIn
					? FSlateColor(FLinearColor::Green)
					: FSlateColor(FLinearColor::Red)
				);
			}
		});
	});
}
