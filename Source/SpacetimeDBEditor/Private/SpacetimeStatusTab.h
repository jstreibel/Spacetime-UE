#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Framework/SlateDelegates.h"
#include "Containers/Ticker.h"

class SSpacetimeStatusTab : public SCompoundWidget
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
	void ScheduleAsyncRefresh() const;
	
	float RefreshInterval= 1.f;
	float AccumulatedTime = 0.f;

	// These are our two colored text widgets
	TSharedPtr<STextBlock> CLITextBlock;
	TSharedPtr<STextBlock> StatusTextBlock;
	
	// Keeps the list alive for the duration of this widget
	TArray<TSharedPtr<FString>> ServerOptions;
	// The currently selected item
	TSharedPtr<FString> SelectedServer;
	// The combo-box itself
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ServerComboBox;

	// Other stuff
	TSharedPtr<SEditableTextBox> DatabaseNameTextBox;
	TSharedPtr<SEditableTextBox> ServerURLTextBox;
};