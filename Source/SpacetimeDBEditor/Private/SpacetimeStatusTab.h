#pragma once

#include "CoreMinimal.h"
#include "CLI/SpacetimeCliConfig.h"
#include "Widgets/SCompoundWidget.h"
#include "Framework/SlateDelegates.h"
#include "Containers/Ticker.h"

class SSpacetimeStatusTab final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSpacetimeStatusTab)
		: _RefreshInterval(5.f)  // default to 5 seconds
		{}

		SLATE_ARGUMENT(float, RefreshInterval)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	void ScheduleAsyncRefresh();
	
	float RefreshInterval= 1.f;
	float AccumulatedTime = 0.f;

	FSpacetimeCliConfig Config;
	
	// These are our two colored text widgets
	TSharedPtr<STextBlock> CliTextBlock;
	TSharedPtr<STextBlock> StatusTextBlock;

	using FStdbServerComboBoxItem = TSharedPtr<FSpacetimeServerConfig>;
	using FStdbServerComboBox = SComboBox<FStdbServerComboBoxItem>; 
	
	// Keeps the list alive for the duration of this widget
	TArray<FStdbServerComboBoxItem> ServerOptions;
	// The currently selected item
	FStdbServerComboBoxItem SelectedServer;
	// The combo-box itself
	TSharedPtr<FStdbServerComboBox> ServerComboBox;

	// Other stuff
	TSharedPtr<SEditableTextBox> DatabaseNameTextBox;
	TSharedPtr<SEditableTextBox> ServerURLTextBox;
};