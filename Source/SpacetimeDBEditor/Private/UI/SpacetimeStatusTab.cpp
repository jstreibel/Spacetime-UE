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

	/*
	* Tight grouping (elements very related): 4 px
	* Standard spacing (most UI):			  8 px
	* Loose grouping (more separation):		 12–16 px
	*/

	constexpr float TightPadding = 4.0;
	constexpr float StandardPadding = 8.0;
	constexpr float LoosePadding = 12.0;
	constexpr float SuperLoosePadding = 16.0;
	
	ChildSlot
	[
		SNew(SVerticalBox)

		// CLI:
		+ SVerticalBox::Slot().AutoHeight().Padding(StandardPadding)
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

		// Status:
		+ SVerticalBox::Slot().AutoHeight().Padding(StandardPadding)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center) [ SNew(STextBlock).Text(FText::FromString(TEXT("Status: "))) ]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center) [ SAssignNew(StatusTextBlock, STextBlock).Text(FText::FromString(TEXT("<placeholder>"))) ]
		]

		// Available servers:
        + SVerticalBox::Slot().AutoHeight().Padding(StandardPadding, LoosePadding) [ SNew(STextBlock).Text(LOCTEXT("ServerSelectLabel", "Available Servers:")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(StandardPadding, TightPadding)
        [
            SAssignNew(ServerComboBox, SComboBox<TSharedPtr<FSpacetimeServerConfig>>)
            .OptionsSource(&ServerOptions)
        	// .OnComboBoxOpening_Lambda([this] { /* ServerComboBox->RefreshOptions(); */ })
            .OnGenerateWidget_Lambda([](FStdbServerComboBoxItem InItem)
            {
                return SNew(STextBlock).Text(InItem.IsValid() ? FText::FromString(InItem->Nickname) : FText::GetEmpty());
            })
            .OnSelectionChanged_Lambda([this](TSharedPtr<FSpacetimeServerConfig> NewValue, ESelectInfo::Type)
            {
                if (NewValue.IsValid() && ServerURITextBox.IsValid()) { ServerURITextBox->SetText(FText::FromString(NewValue->GetURI())); }
            })
            [
                // what’s shown when collapsed
                SNew(STextBlock)
                .Text_Lambda([this]()
                {
                    if (ServerComboBox.IsValid() && ServerComboBox->GetSelectedItem().IsValid())
                    {
                    	const auto SelectedServer = ServerComboBox->GetSelectedItem();
                        return FText::FromString(SelectedServer->Nickname);
                    }
                    return LOCTEXT("NoServer", "No servers");
                })
            ]
        ]

        // Selected server URL:
        + SVerticalBox::Slot().AutoHeight().Padding(StandardPadding, StandardPadding)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("DBUrl", "SpacetimeDB instance URI:"))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(StandardPadding, StandardPadding)
        [
            SAssignNew(ServerURITextBox, SEditableTextBox)
            .Text(SelectedServer.IsValid()
                      ? FText::FromString(SelectedServer->GetURI())
                      : LOCTEXT("URLPlaceholder", "http://localhost.com:3000"))
            .HintText(LOCTEXT("DBServerURLHint", "e.g. http://localhost.com:3000"))
        ]
		
		// Database Name:
        + SVerticalBox::Slot().AutoHeight().Padding(StandardPadding, StandardPadding)
        [ SNew(STextBlock).Text(LOCTEXT("DBNameLabel", "Database Name:")) ]
        + SVerticalBox::Slot().AutoHeight().Padding(StandardPadding, StandardPadding)
        [ SAssignNew(DatabaseNameTextBox, SEditableTextBox).HintText(LOCTEXT("DBNameHint", "e.g. quickstart-chat")) ]

		// Buttons:
        + SVerticalBox::Slot().AutoHeight().Padding(StandardPadding, StandardPadding)
        [
	        SNew(SHorizontalBox)

	        // Generate Code Reflection
	        + SHorizontalBox::Slot().AutoWidth().Padding(StandardPadding)
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

					if (!ServerURITextBox.IsValid())
					{
						FMessageDialog::Open(
							EAppMsgCategory::Error,
							EAppMsgType::Ok,
							LOCTEXT("STDBGenerateMissedServerURL",
								"🛑 Missing field: server URL"));
						return FReply::Handled();
					}

					// 2) Grab the text out of the widgets
					const FString ServerURL = ServerURITextBox->GetText().ToString();
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

        	+ SHorizontalBox::Slot().AutoWidth().Padding(StandardPadding)
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshButton", "Refresh"))
				.OnClicked_Lambda([this]() -> FReply
				{
					ScheduleAsyncRefresh();
					return FReply::Handled();
				})
			]
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
		// ScheduleAsyncRefresh();
	}
}

void SSpacetimeStatusTab::ScheduleAsyncRefresh() {
	// Capture weak refs so we don’t keep widgets alive if the tab closes
	TWeakPtr<STextBlock> CliWeak    = CliTextBlock;
	TWeakPtr<STextBlock> StatusWeak = StatusTextBlock;

	// Dispatch the blocking work to the thread-pool
	Async(EAsyncExecution::ThreadPool, [this, CliWeak, StatusWeak]()
	{		
		// *** Blocking calls go here ***
		const bool bCliAvailable = FSpacetimeCLIHelper::IsCliAvailable();
		const bool bLoggedIn     = FSpacetimeCLIHelper::IsLoggedIn();
		
		// Then marshal back to the game (main) thread
		// TODO: make all args weak
		AsyncTask(ENamedThreads::GameThread, [this, CliWeak, StatusWeak, bCliAvailable, bLoggedIn]()
		{
			if (const auto CliBlock = CliWeak.Pin())
			{
				CliBlock->SetText(FText::FromString(bCliAvailable ? TEXT("available")   : TEXT("unavailable")));
				CliBlock->SetColorAndOpacity(bCliAvailable
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

			ServerOptions.Empty();
			ServerURITextBox->SetText(FText::FromString(TEXT("(no servers)")));
			
			// --- 1) load CLI config and build combo‐list ---
			if (bCliAvailable && bLoggedIn)
			{
				if (FString ConfigError; FSpacetimeCLIHelper::GetCliConfig(Config, ConfigError))
				{
					for (const auto& ServerConfig : Config.ServerConfigs)
					{
						ServerOptions.Add(MakeShared<FSpacetimeServerConfig>(ServerConfig));
					}

					if (ServerOptions.Num() > 0)
					{
						// TODO: select default server from 'cli.toml'
						SelectedServer = ServerOptions[0];
						ServerURITextBox->SetText(FText::FromString(SelectedServer->GetURI()));
					}
				}
				else
				{
					// optional: log or show ConfigError
					UE_LOG(LogTemp, Warning, TEXT("Failed to load cli.toml: %s"), *ConfigError);
				}
			}

			ServerComboBox->RefreshOptions();
			ServerComboBox->SetSelectedItem(SelectedServer);
		});
	});
}
