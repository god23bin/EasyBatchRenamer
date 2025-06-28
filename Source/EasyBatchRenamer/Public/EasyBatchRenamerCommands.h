// Copyright Devil Dev, 2025.

#pragma once

#include "Framework/Commands/Commands.h"
#include "EasyBatchRenamerStyle.h"

class FEasyBatchRenamerCommands : public TCommands<FEasyBatchRenamerCommands>
{
public:

	FEasyBatchRenamerCommands()
		: TCommands<FEasyBatchRenamerCommands>(TEXT("EasyBatchRenamer"), NSLOCTEXT("Contexts", "EasyBatchRenamer", "EasyBatchRenamer Plugin"), NAME_None, FEasyBatchRenamerStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};