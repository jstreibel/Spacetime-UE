#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SCompoundWidget.h"

class FSpacetimeDBEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TSharedRef<SDockTab> SpawnGeneratorTab(const class FSpawnTabArgs& Args);

    // Keep pointers to text boxes so we can read their values in the button callback
    TSharedPtr<SEditableTextBox> DatabaseNameTextBox;
    TSharedPtr<SEditableTextBox> ServerURLTextBox;
    
};
