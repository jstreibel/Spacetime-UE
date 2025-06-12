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

	// --- 1) load CLI config and build combo‐list ---

	if (FString ConfigError; FSpacetimeCLIHelper::GetCliConfig(Config, ConfigError))
	{
		for (const auto& ServerConfig : Config.ServerConfigs)
		{
			ServerOptions.Add(MakeShared<FString>(ServerConfig.Nickname));
		}

		if (ServerOptions.Num() > 0)
		{			
			SelectedServer = ServerOptions[0];
		}
	}
	else
	{
		// optional: log or show ConfigError
		UE_LOG(LogTemp, Warning, TEXT("Failed to load cli.toml: %s"), *ConfigError);
	}
	
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
				SAssignNew(CliTextBlock, STextBlock)
				.Text(FText::FromString(TEXT("<placeholder>")))
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
				.Text(FText::FromString(TEXT("<placeholder>")))
			]
		]

		// --- Dropdown label ---
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ServerSelectLabel", "Select Server:"))
        ]

        // --- The combo box itself ---
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SAssignNew(ServerComboBox, SComboBox<TSharedPtr<FString>>)
            .OptionsSource(&ServerOptions)
            .InitiallySelectedItem(SelectedServer)
            .OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
            {
                return SNew(STextBlock)
                    .Text(InItem.IsValid() ? FText::FromString(*InItem) : FText::GetEmpty());
            })
            .OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewValue, ESelectInfo::Type)
            {
                if (NewValue.IsValid() && ServerURLTextBox.IsValid())
                {
                    ServerURLTextBox->SetText(FText::FromString(*NewValue));
                }
            })
            [
                // what’s shown when collapsed
                SNew(STextBlock)
                .Text_Lambda([this]()
                {
                    if (ServerComboBox.IsValid() && ServerComboBox->GetSelectedItem().IsValid())
                    {
                        return FText::FromString(*ServerComboBox->GetSelectedItem());
                    }
                    return LOCTEXT("NoServer", "No servers");
                })
            ]
        ]

        // --- URL + DB name + Generate button (unchanged) ---
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("DBUrl", "Server URL:"))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SAssignNew(ServerURLTextBox, SEditableTextBox)
            .Text(SelectedServer.IsValid()
                      ? FText::FromString(*SelectedServer)
                      : LOCTEXT("URLPlaceholder", "http://localhost.com:3000"))
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
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SButton)
            .Text(LOCTEXT("GenerateButton", "Generate Code Reflection"))
        	.OnClicked_Lambda([this]() -> FReply
        	{
        		// 1) Ensure the fields are valid
				if (!DatabaseNameTextBox.IsValid())
				{
					FMessageDialog::Open(
						EAppMsgCategory::Error,
						EAppMsgType::Ok,
						LOCTEXT("STDBGenerateMissedDatabaseName",
							"🛑 Missing field: database name"));
					return FReply::Handled();
				}

				if (!ServerURLTextBox.IsValid())
				{
					FMessageDialog::Open(
						EAppMsgCategory::Error,
						EAppMsgType::Ok,
						LOCTEXT("STDBGenerateMissedServerURL",
							"🛑 Missing field: server URL"));
					return FReply::Handled();
				}

				// 2) Grab the text out of the widgets
				const FString ServerURL = ServerURLTextBox->GetText().ToString();
				const FString DBName    = DatabaseNameTextBox->GetText().ToString();

				// 3) Call into your code-gen helper
				FString OutDir, ErrorMsg;
				const bool bOK = USpacetimeDBEditorHelpers::GenerateCxxUnrealCodeFromSpacetimeDB(
					ServerURL, DBName, OutDir, ErrorMsg
				);

				// 4) Show a dialog based on success or failure
				if (!bOK)
				{
					FMessageDialog::Open(
						EAppMsgCategory::Error,
						EAppMsgType::Ok,
						FText::Format(
							LOCTEXT("GenerateFailedFmt", "🛑 Generate failed:\n{0}"),
							FText::FromString(ErrorMsg)
						)
					);
				}
				else
				{
					FMessageDialog::Open(
						EAppMsgCategory::Success,
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
	TWeakPtr<STextBlock> CliWeak    = CliTextBlock;
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
