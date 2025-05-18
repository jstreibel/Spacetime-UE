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
    TSharedRef<class SDockTab> SpawnGeneratorTab(const class FSpawnTabArgs& Args);

    // Keep a pointer to the text box so we can read its value in the button callback
    TSharedPtr<class SEditableTextBox> DatabaseNameTextBox;
};
