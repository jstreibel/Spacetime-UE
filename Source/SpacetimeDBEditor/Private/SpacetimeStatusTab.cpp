#include "SpacetimeStatusTab.h"

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
	FSpacetimeCliConfig Config;
	FString ConfigError;

	if (FSpacetimeCLIHelper::GetCliConfig(Config, ConfigError))
	{
		for (const auto& Cfg : Config.ServerConfigs)
		{
			// you could show Nickname instead, e.g. Cfg.Nickname, or combine both
			ServerOptions.Add(MakeShared<FString>(Cfg.Host));
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
            // … your existing OnClicked lambda …
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
