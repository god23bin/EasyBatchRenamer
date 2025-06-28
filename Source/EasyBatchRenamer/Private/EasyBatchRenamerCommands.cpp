// Copyright Devil Dev, 2025.

#include "EasyBatchRenamerCommands.h"

#define LOCTEXT_NAMESPACE "FEasyBatchRenamerModule"

void FEasyBatchRenamerCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "EasyBatchRenamer", "Bring up EasyBatchRenamer window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
